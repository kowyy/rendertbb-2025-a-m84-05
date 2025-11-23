#ifndef RENDER_CONFIG_HPP
#define RENDER_CONFIG_HPP

#include "vector.hpp"
#include <cstdint>
#include <string>

namespace render {

  // Almacena la configuración para el renderizado
  class config {
  public:
    config() = default;

    // Getters para parámetros de imagen
    [[nodiscard]] int get_aspect_width() const { return aspect_w; }

    [[nodiscard]] int get_aspect_height() const { return aspect_h; }

    [[nodiscard]] int get_image_width() const { return image_width; }

    [[nodiscard]] double get_gamma() const { return gamma; }

    // Getters para parámetros de la cámara
    [[nodiscard]] vector get_camera_position() const { return camera_position; }

    [[nodiscard]] vector get_camera_target() const { return camera_target; }

    [[nodiscard]] vector get_camera_north() const { return camera_north; }

    [[nodiscard]] double get_field_of_view() const { return field_of_view; }

    // Getters para parámetros de ray tracing
    [[nodiscard]] int get_samples_per_pixel() const { return samples_per_pixel; }

    [[nodiscard]] int get_max_depth() const { return max_depth; }

    [[nodiscard]] std::uint64_t get_material_rng_seed() const { return material_rng_seed; }

    [[nodiscard]] std::uint64_t get_ray_rng_seed() const { return ray_rng_seed; }

    // Getters para colores de fondo
    [[nodiscard]] vector get_background_dark_color() const { return background_dark_color; }

    [[nodiscard]] vector get_background_light_color() const { return background_light_color; }

    // Setters con validación
    void set_aspect_ratio(int width, int height);
    void set_image_width(int width);
    void set_gamma(double g);
    void set_camera_position(vector const & pos);
    void set_camera_target(vector const & target);
    void set_camera_north(vector const & north);
    void set_field_of_view(double fov);
    void set_samples_per_pixel(int samples);
    void set_max_depth(int depth);
    void set_material_rng_seed(std::uint64_t seed);
    void set_ray_rng_seed(std::uint64_t seed);
    void set_background_dark_color(vector const & color);
    void set_background_light_color(vector const & color);

  private:
    // Parámetros de imagen
    int aspect_w{16};
    int aspect_h{9};
    int image_width{1'920};
    double gamma{2.2};

    // Parámetros de cámara
    vector camera_position{0.0, 0.0, -10.0};
    vector camera_target{0.0, 0.0, 0.0};
    vector camera_north{0.0, 1.0, 0.0};

    // Parámetros de ray tracing
    double field_of_view{90.0};
    int samples_per_pixel{20};
    int max_depth{5};

    // Semillas para generación de números aleatorios
    std::uint64_t material_rng_seed{13};
    std::uint64_t ray_rng_seed{19};

    // Colores de fondo para el gradiente
    vector background_dark_color{0.25, 0.5, 1.0};
    vector background_light_color{1.0, 1.0, 1.0};
  };

  // Carga configuración desde archivo de texto
  void load_config(std::string const & path, config & out);

}  // namespace render

#endif
