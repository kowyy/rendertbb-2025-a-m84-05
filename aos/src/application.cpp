#include "application.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "config.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"
#include "vector.hpp"
#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <gsl/span>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

  // Contiene toda la información necesaria para el renderizado
  struct RenderJob {
    render::config cfg;
    render::scene scene_data;
    render::camera cam;
    std::string output_path;
    std::mt19937_64 ray_rng;
    std::mt19937_64 material_rng;

    RenderJob(std::string const & config_path, std::string const & scene_path,
              std::string output_path_p)
        : cam{cfg}, output_path{std::move(output_path_p)} {
      render::load_config(config_path, cfg);
      render::parse_scene_file(scene_path, scene_data);

      cam = render::camera{cfg};

      // Inicializar generadores de números aleatorios
      ray_rng.seed(static_cast<std::mt19937_64::result_type>(cfg.get_ray_rng_seed()));
      material_rng.seed(static_cast<std::mt19937_64::result_type>(cfg.get_material_rng_seed()));
    }
  };

  // Parámetros para renderizado de píxeles
  struct PixelRenderParams {
    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;
    std::uniform_real_distribution<double> * dist;
  };

  // Parámetros para guardado de imagen
  struct ImageSaveParams {
    int width;
    int height;
    double gamma;
  };

  // Calcula color de un rayo recursivamente con rebotes
  render::color ray_color(render::ray const & r, RenderJob const & job, int depth,
                          std::mt19937_64 & mat_rng) {
    // Si alcanzamos profundidad máxima, devolver negro
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
          // Calcular color del rayo dispersado
          render::color const recursive_color = ray_color(scattered, job, depth - 1, mat_rng);
          return render::color{result.attenuation} * recursive_color;
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

  // Renderiza un píxel con múltiples muestras
  render::color render_pixel(int i, int j, RenderJob & job, PixelRenderParams const & params) {
    render::color accumulated{0.0, 0.0, 0.0};

    // Generar múltiples rayos con posiciones aleatorias dentro del píxel
    for (int s = 0; s < params.samples_per_pixel; ++s) {
      auto const u =
          (static_cast<double>(i) + 0.5 + (*params.dist)(job.ray_rng)) / params.image_width;
      auto const v =
          (static_cast<double>(j) + 0.5 + (*params.dist)(job.ray_rng)) / params.image_height;

      render::ray const ray_sample = job.cam.get_ray(u, v);
      accumulated += ray_color(ray_sample, job, params.max_depth, job.material_rng);
    }

    return accumulated / static_cast<double>(params.samples_per_pixel);
  }

  // Guarda imagen en formato PPM
  void save_ppm(std::string const & filename, std::vector<render::color> const & image,
                ImageSaveParams const & params) {
    std::ofstream out(filename);
    if (!out.is_open()) {
      throw std::runtime_error("Error: Cannot open file for writing: " + filename);
    }

    out << "P3\n" << params.width << " " << params.height << "\n255\n";

    for (int j = 0; j < params.height; ++j) {
      for (int i = 0; i < params.width; ++i) {
        size_t const index =
            static_cast<size_t>(j) * static_cast<size_t>(params.width) + static_cast<size_t>(i);
        render::color const & pixel = image[index];

        auto const r = pixel.to_discrete_r(params.gamma);
        auto const g = pixel.to_discrete_g(params.gamma);
        auto const b = pixel.to_discrete_b(params.gamma);

        out << static_cast<int>(r) << " " << static_cast<int>(g) << " " << static_cast<int>(b)
            << "\n";
      }
    }
  }

  // Bucle principal de renderizado en AOS
  void render_loop(RenderJob & job) {
    int const image_width = job.cfg.get_image_width();
    auto const aspect_ratio =
        static_cast<double>(job.cfg.get_aspect_width()) / job.cfg.get_aspect_height();
    int const image_height = static_cast<int>(image_width / aspect_ratio);

    // Distribución para antialiasing
    std::uniform_real_distribution<double> dist(-0.5, 0.5);

    PixelRenderParams const render_params{
      image_width, image_height, job.cfg.get_samples_per_pixel(), job.cfg.get_max_depth(), &dist};

    ImageSaveParams const save_params{image_width, image_height, job.cfg.get_gamma()};

    // Almacenar imagen completa en memoria (AOS)
    std::vector<render::color> image(static_cast<size_t>(image_width) *
                                     static_cast<size_t>(image_height));

    std::cout << "Renderizando escena (" << image_width << "x" << image_height << ") con "
              << render_params.samples_per_pixel << " samples/pixel...\n";

    // Renderizar fila por fila
    for (int j = 0; j < image_height; ++j) {
      std::cerr << "\rScanlines restantes: " << (image_height - j) << "   " << std::flush;

      for (int i = 0; i < image_width; ++i) {
        render::color const pixel_color = render_pixel(i, j, job, render_params);

        size_t const index =
            static_cast<size_t>(j) * static_cast<size_t>(image_width) + static_cast<size_t>(i);
        image[index] = pixel_color;
      }
    }

    std::cerr << "\rRenderizado completado.                    \n";

    save_ppm(job.output_path, image, save_params);
    std::cout << "Imagen guardada como " << job.output_path << "\n";
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

    return EXIT_SUCCESS;
  } catch (std::exception const & e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
