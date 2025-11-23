#include "application.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "config.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"
#include "vector.hpp"

#include <oneapi/tbb/partitioner.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <gsl/gsl>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
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
    std::string output_path;

    // Semillas y almacenamiento TLS
    std::vector<std::uint64_t> ray_seeds;
    std::vector<std::uint64_t> material_seeds;
    tbb::enumerable_thread_specific<std::mt19937_64> ray_rngs;
    tbb::enumerable_thread_specific<std::mt19937_64> material_rngs;

    RenderJob(std::string const & config_path, std::string const & scene_path,
              std::string output_path_p)
    : cam{cfg}, output_path{std::move(output_path_p)} {

        render::load_config(config_path, cfg);
        render::parse_scene_file(scene_path, scene_data);
        cam = render::camera{cfg};

        // Inicialización de semillas
        size_t const num_seeds = 256;
        ray_seeds.resize(num_seeds);
        material_seeds.resize(num_seeds);

        std::mt19937_64 master_ray_gen{
            static_cast<std::mt19937_64::result_type>(cfg.get_ray_rng_seed())};
        std::ranges::generate(ray_seeds, master_ray_gen);

        std::mt19937_64 master_mat_gen{
            static_cast<std::mt19937_64::result_type>(cfg.get_material_rng_seed())};
        std::ranges::generate(material_seeds, master_mat_gen);

        // Inicialización TLS
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

struct PixelRenderParams {
    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;
};

struct ImageSaveParams {
    int width;
    int height;
    double gamma;
};

class PixelRenderer {
public:
    PixelRenderer(RenderJob const * j, PixelRenderParams const * p)
    : job(j), params(p) {}

    // Renderiza un píxel completo
    render::color render_pixel(int i, int j, ThreadLocalRNGs & rngs) const {
        render::color accumulated{0.0, 0.0, 0.0};
        std::uniform_real_distribution<double> dist(-0.5, 0.5);

        for (int s = 0; s < params->samples_per_pixel; ++s) {
            accumulated += compute_sample_color(i, j, rngs, dist);
        }
        return accumulated / static_cast<double>(params->samples_per_pixel);
    }

private:
    RenderJob const * job;
    PixelRenderParams const * params;

    // Calcula el color de un rayo
    render::color
    compute_sample_color(int i, int j, ThreadLocalRNGs & rngs,
                         std::uniform_real_distribution<double> & dist) const {
        auto const u =
            (static_cast<double>(i) + 0.5 + dist(*rngs.ray)) / params->image_width;
        auto const v =
            (static_cast<double>(j) + 0.5 + dist(*rngs.ray)) / params->image_height;

        render::ray const ray_sample = job->cam.get_ray(u, v);
        return trace_ray(ray_sample, params->max_depth, *rngs.material);
    }

    // Función recursiva de trazado de rayos
    render::color trace_ray(render::ray const & r, int depth,
                            std::mt19937_64 & mat_rng) const {
        if (depth <= 0) {
            return render::color{0.0, 0.0, 0.0};
        }

        render::hit_record rec;
        constexpr double min_t = 1e-3;

        if (job->scene_data.hit(r, min_t, std::numeric_limits<double>::infinity(),
                                rec)) {
            render::ray scattered;
            if (rec.mat_ptr != nullptr) {
                auto const result = rec.mat_ptr->scatter(r, rec, scattered, mat_rng);
                if (result.scattered) {
                    return render::color{result.attenuation} *
                    trace_ray(scattered, depth - 1, mat_rng);
                }
            }
            return render::color{0.0, 0.0, 0.0};
        }

        render::vector const unit_direction = r.get_direction().normalized();
        auto const t                        = 0.5 * (unit_direction.y + 1.0);
        return render::color{(1.0 - t) * job->cfg.get_background_light_color() +
            t * job->cfg.get_background_dark_color()};
    }
};

void save_ppm(std::string const & filename,
              std::vector<render::color> const & image,
              ImageSaveParams const & params) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Error: Cannot open file for writing: " + filename);
    }

    out << "P3\n" << params.width << " " << params.height << "\n255\n";

    for (int j = 0; j < params.height; ++j) {
        for (int i = 0; i < params.width; ++i) {
            size_t const index = static_cast<size_t>(j) *
                static_cast<size_t>(params.width) +
                static_cast<size_t>(i);
            render::color const & pixel = image[index];

            out << static_cast<int>(pixel.to_discrete_r(params.gamma)) << " "
                << static_cast<int>(pixel.to_discrete_g(params.gamma)) << " "
                << static_cast<int>(pixel.to_discrete_b(params.gamma)) << "\n";
        }
    }
}

void execute_parallel_render(RenderJob & job,
                             PixelRenderParams const & render_params,
                             std::vector<render::color> & image) {
    PixelRenderer const renderer(&job, &render_params);

    int const image_height = render_params.image_height;
    int const image_width  = render_params.image_width;

    tbb::parallel_for(
        tbb::blocked_range<int>(0, image_height),
        [&](tbb::blocked_range<int> const & r) {
            // Punteros a los generadores locales
            ThreadLocalRNGs local_rngs{&job.ray_rngs.local(),
                &job.material_rngs.local()};

            for (int j = r.begin(); j != r.end(); ++j) {
                for (int i = 0; i < image_width; ++i) {
                    render::color const pixel_color =
                        renderer.render_pixel(i, j, local_rngs);

                    size_t const index = static_cast<size_t>(j) *
                        static_cast<size_t>(image_width) +
                        static_cast<size_t>(i);
                    image[index] = pixel_color;
                }
            }
        tbb::auto_partitioner();
        });
}

void render_loop(RenderJob & job) {
    int const image_width = job.cfg.get_image_width();
    auto const aspect_ratio = static_cast<double>(job.cfg.get_aspect_width()) /
        job.cfg.get_aspect_height();
    int const image_height = static_cast<int>(image_width / aspect_ratio);

    PixelRenderParams const render_params{image_width, image_height,
        job.cfg.get_samples_per_pixel(),
        job.cfg.get_max_depth()};

    ImageSaveParams const save_params{image_width, image_height,
        job.cfg.get_gamma()};

    std::vector<render::color> image(static_cast<size_t>(image_width) *
                                     static_cast<size_t>(image_height));

    std::cout << "Renderizando escena (" << image_width << "x" << image_height
        << ") con TBB...\n";

    execute_parallel_render(job, render_params, image);

    std::cout << "Renderizado completado.\n";
    save_ppm(job.output_path, image, save_params);
    std::cout << "Imagen guardada como " << job.output_path << "\n";
}

} // namespace

int render::Application::run(gsl::span<char const *const> args) {
    if (args.size() != 4) {
        std::cerr << "Error: Invalid number of arguments: " << args.size() - 1
            << '\n';
        return EXIT_FAILURE;
    }

    try {
        RenderJob job(args[1], args[2], args[3]);

        auto const start_time = std::chrono::high_resolution_clock::now();
        render_loop(job);
        auto const end_time = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> const elapsed = end_time - start_time;
        std::cout << "Tiempo total: " << elapsed.count() << " segundos.\n";

        return EXIT_SUCCESS;
    } catch (std::exception const & e) {
        std::cerr << "Excepción capturada: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
