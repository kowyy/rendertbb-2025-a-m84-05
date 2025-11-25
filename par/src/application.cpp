#include "application.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "config.hpp"
#include "image_soa_par.hpp" 
#include "object.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"
#include "vector.hpp"

#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/partitioner.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/global_control.h>

#include <algorithm>
#include <functional>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <gsl/span>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace {

  struct ThreadLocalRNGs {
      std::mt19937_64 * ray;
      std::mt19937_64 * material;
  };

  struct RenderJob {
    render::config cfg;
    render::scene scene_data;
    render::camera cam;
    ImageSOA image;
    std::string output_path;

    std::vector<std::uint64_t> ray_seeds;
    std::vector<std::uint64_t> material_seeds;
    tbb::enumerable_thread_specific<std::mt19937_64> ray_rngs;
    tbb::enumerable_thread_specific<std::mt19937_64> material_rngs;

    RenderJob(std::string const & config_path, std::string const & scene_path,
              std::string output_path_p)
        : cam{cfg}, image{1, 1}, output_path(std::move(output_path_p)) {
      
      load_resources(config_path, scene_path);
      init_rngs();
    }

  private:
    void load_resources(std::string const & config_path, std::string const & scene_path) {
      render::load_config(config_path, cfg);
      render::parse_scene_file(scene_path, scene_data);

      int const image_width = cfg.get_image_width();
      auto const aspect_ratio =
          static_cast<double>(cfg.get_aspect_width()) / cfg.get_aspect_height();
      int const image_height = static_cast<int>(image_width / aspect_ratio);

      cam = render::camera{cfg};
      image = ImageSOA{image_width, image_height};
    }

    void init_rngs() {
      size_t const num_seeds = 256; 
      ray_seeds.resize(num_seeds);
      material_seeds.resize(num_seeds);

    std::mt19937_64 master_ray_gen{
        static_cast<std::mt19937_64::result_type>(cfg.get_ray_rng_seed())};
    std::ranges::generate(ray_seeds, std::ref(master_ray_gen));

    std::mt19937_64 master_mat_gen{
        static_cast<std::mt19937_64::result_type>(cfg.get_material_rng_seed())};
    std::ranges::generate(material_seeds, std::ref(master_mat_gen));

      ray_rngs = tbb::enumerable_thread_specific<std::mt19937_64>{[this] {
          static std::atomic<size_t> counter{0};
          size_t const idx = counter++ % this->ray_seeds.size();
          return std::mt19937_64{this->ray_seeds[idx]};
      }};

      material_rngs = tbb::enumerable_thread_specific<std::mt19937_64>{[this] {
          static std::atomic<size_t> counter{0};
          size_t const idx = counter++ % this->material_seeds.size();
          return std::mt19937_64{this->material_seeds[idx]};
      }};
    }
  };

  // Calcula color de un rayo recursivamente
  render::color ray_color(render::ray const & r, RenderJob const & job, int depth,
                          std::mt19937_64 & mat_rng) {
    if (depth <= 0) {
      return render::color{0.0, 0.0, 0.0};
    }

    render::hit_record rec;
    constexpr double min_t = 1e-3;

    if (job.scene_data.hit(r, min_t, std::numeric_limits<double>::infinity(), rec)) {
      render::ray scattered;
      if (rec.mat_ptr != nullptr) {
        auto const result = rec.mat_ptr->scatter(r, rec, scattered, mat_rng);
        if (result.scattered) {
          return render::color{result.attenuation} * ray_color(scattered, job, depth - 1, mat_rng);
        }
      }
      return render::color{0.0, 0.0, 0.0};
    }

    render::vector const unit_direction = r.get_direction().normalized();
    auto const t = 0.5 * (unit_direction.y + 1.0);
    return render::color{(1.0 - t) * job.cfg.get_background_light_color() +
                         t * job.cfg.get_background_dark_color()};
  }

  class RenderTask {
    RenderJob * job;
    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;
    double gamma;

  public:
    explicit RenderTask(RenderJob * j)
      : job(j),
        image_width(j->image.get_width()),
        image_height(j->image.get_height()),
        samples_per_pixel(j->cfg.get_samples_per_pixel()),
        max_depth(j->cfg.get_max_depth()),
        gamma(j->cfg.get_gamma()) {}

    void operator()(tbb::blocked_range<int> const & r) const {
      ThreadLocalRNGs const local_rngs{
          &job->ray_rngs.local(),
          &job->material_rngs.local()
      };
      
      std::uniform_real_distribution<double> dist(-0.5, 0.5);

      for (int j = r.begin(); j != r.end(); ++j) {
        for (int i = 0; i < image_width; ++i) {
          render::color accumulated{0.0, 0.0, 0.0};

          for (int s = 0; s < samples_per_pixel; ++s) {
            auto const u = (static_cast<double>(i) + 0.5 + dist(*local_rngs.ray)) / image_width;
            auto const v = (static_cast<double>(j) + 0.5 + dist(*local_rngs.ray)) / image_height;

            render::ray const ray_sample = job->cam.get_ray(u, v);
            accumulated += ray_color(ray_sample, *job, max_depth, *local_rngs.material);
          }

          render::color const pixel_color = accumulated / static_cast<double>(samples_per_pixel);
          job->image.set_pixel(i, j, pixel_color, gamma);
        }
      }
    }
  };

  // Función auxiliar para configurar TBB
  std::unique_ptr<tbb::global_control> setup_tbb(render::config const & cfg) {
    int const n_threads = cfg.get_num_threads();
    if (n_threads > 0) {
      std::cout << "Configuración TBB: Limitando a " << n_threads << " hilos.\n";
      return std::make_unique<tbb::global_control>(
          tbb::global_control::max_allowed_parallelism,
          static_cast<size_t>(n_threads)
      );
    }
    std::cout << "Configuración TBB: Automático (todos los núcleos).\n";
    return nullptr;
  }

  void render_loop(RenderJob & job) {
    auto global_limit = setup_tbb(job.cfg);

    int const width = job.image.get_width();
    int const height = job.image.get_height();

    std::cout << "Renderizando escena (" << width << "x" << height 
              << ") con TBB...\n";

    RenderTask const task(&job);
    
    std::string const part_type = job.cfg.get_partitioner();
    int const grain = job.cfg.get_grain_size();
    tbb::blocked_range<int> const range(0, height, static_cast<size_t>(grain));

    if (part_type == "static") {
      tbb::parallel_for(range, task, tbb::static_partitioner());
    } else if (part_type == "simple") {
      tbb::parallel_for(range, task, tbb::simple_partitioner());
    } else {
      tbb::parallel_for(range, task, tbb::auto_partitioner());
    }

    std::cout << "Renderizado completado.\n";
  }

} // namespace

int render::Application::run(gsl::span<char const * const> args) {
  if (args.size() != 4) {
    std::cerr << "Error: Invalid number of arguments: " << args.size() - 1 << '\n';
    return EXIT_FAILURE;
  }

  try {
    RenderJob job(args[1], args[2], args[3]);

    auto const start_time = std::chrono::high_resolution_clock::now();
    render_loop(job);
    auto const end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> const elapsed = end_time - start_time;
    std::cout << "Tiempo total: " << elapsed.count() << " segundos.\n";

    job.image.save_ppm(job.output_path);
    std::cout << "Imagen guardada como " << job.output_path << "\n";

    return EXIT_SUCCESS;
  } catch (std::exception const & e) {
    std::cerr << "Excepción: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
