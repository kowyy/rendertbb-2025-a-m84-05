#include "application.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "config.hpp"
#include "image_soa.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"
#include "vector.hpp"
#include <chrono>
#include <cstdlib>
#include <exception>
#include <gsl/span>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <utility>

namespace {

  // Contiene toda la información necesaria para el renderizado
  struct RenderJob {
    render::config cfg;
    render::scene scene_data;
    render::camera cam;
    ImageSOA image;
    std::string output_path;
    std::mt19937_64 ray_rng;
    std::mt19937_64 material_rng;

    RenderJob(std::string const & config_path, std::string const & scene_path,
              std::string output_path_p)
        : cam{cfg},
          image{
            cfg.get_image_width(),
            static_cast<int>(cfg.get_image_width() / (static_cast<double>(cfg.get_aspect_width()) /
                                                      cfg.get_aspect_height()))},
          output_path(std::move(output_path_p)) {
      render::load_config(config_path, cfg);
      render::parse_scene_file(scene_path, scene_data);

      // Calcular dimensiones reales de la imagen
      int const image_width = cfg.get_image_width();
      auto const aspect_ratio =
          static_cast<double>(cfg.get_aspect_width()) / cfg.get_aspect_height();
      int const image_height = static_cast<int>(image_width / aspect_ratio);

      // Re-crear cámara e imagen con configuración cargada
      cam   = render::camera{cfg};
      image = ImageSOA{image_width, image_height};

      // Inicializar generadores de números aleatorios
      ray_rng.seed(static_cast<std::mt19937_64::result_type>(cfg.get_ray_rng_seed()));
      material_rng.seed(static_cast<std::mt19937_64::result_type>(cfg.get_material_rng_seed()));
    }
  };

  // Calcula color de un rayo recursivamente con rebotes
  render::color ray_color(render::ray const & r, RenderJob const & job, int depth,
                          std::mt19937_64 & mat_rng) {
    // Si alcanzamos profundidad máxima, devolver negro
    if (depth <= 0) {
      return render::color{0.0, 0.0, 0.0};
    }

    render::hit_record rec;

    constexpr double min_t = 1e-8;
    if (job.scene_data.hit(r, min_t, std::numeric_limits<double>::infinity(), rec)) {
      render::ray scattered;

      if (rec.mat_ptr != nullptr) {
        auto const result = rec.mat_ptr->scatter(r, rec, scattered, mat_rng);

        if (result.scattered) {
          // Calcular color del rayo dispersado
          return render::color{result.attenuation} * ray_color(scattered, job, depth - 1, mat_rng);
        }
      }

      return render::color{0.0, 0.0, 0.0};
    }

    // Devolver gradiente de fondo
    render::vector const unit_direction = r.get_direction().normalized();
    auto const t                        = 0.5 * (unit_direction.y + 1.0);
    return render::color{(1.0 - t) * job.cfg.get_background_light_color() +
                         t * job.cfg.get_background_dark_color()};
  }

  // Bucle principal de renderizado (SOA)
  void render_loop(RenderJob & job) {
    int const image_width       = job.image.get_width();
    int const image_height      = job.image.get_height();
    int const samples_per_pixel = job.cfg.get_samples_per_pixel();
    int const max_depth         = job.cfg.get_max_depth();
    double const gamma          = job.cfg.get_gamma();

    // Distribución para antialiasing
    std::uniform_real_distribution<double> dist(-0.5, 0.5);

    std::cout << "Renderizando escena (" << image_width << "x" << image_height << ") con "
              << samples_per_pixel << " samples/pixel...\n";

    // Renderizar fila por fila
    for (int j = 0; j < image_height; ++j) {
      std::cerr << "\rScanlines restantes: " << (image_height - j) << "   " << std::flush;

      for (int i = 0; i < image_width; ++i) {
        render::color accumulated{0.0, 0.0, 0.0};

        // Generar múltiples rayos con posiciones aleatorias
        for (int s = 0; s < samples_per_pixel; ++s) {
          auto const u = (static_cast<double>(i) + 0.5 + dist(job.ray_rng)) / image_width;
          auto const v = (static_cast<double>(j) + 0.5 + dist(job.ray_rng)) / image_height;

          render::ray const r              = job.cam.get_ray(u, v);
          render::color const sample_color = ray_color(r, job, max_depth, job.material_rng);
          accumulated += sample_color;
        }

        // Promediar muestras y guardar píxel directamente (SOA)
        render::color const pixel_color = accumulated / static_cast<double>(samples_per_pixel);
        job.image.set_pixel(i, j, pixel_color, gamma);
      }
    }

    std::cerr << "\rRenderizado completado. \n";
  }

}  // namespace

// Punto de entrada principal de la aplicación
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

    // Guardar imagen al final en canales separados para SOA
    job.image.save_ppm(job.output_path);
    std::cout << "Imagen guardada como " << job.output_path << "\n";

    return EXIT_SUCCESS;
  } catch (std::exception const & e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
