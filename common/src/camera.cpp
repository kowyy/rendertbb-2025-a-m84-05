#include "camera.hpp"
#include "config.hpp"
#include "vector.hpp"
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace render {

  namespace {

    constexpr double degrees_to_radians(double degrees) {
      return degrees * std::numbers::pi / 180.0;
    }

  }  // namespace

  // Construye la cámara
  camera::camera(config const & cfg) {
    double const aspect_ratio =
        static_cast<double>(cfg.get_aspect_width()) / static_cast<double>(cfg.get_aspect_height());
    int const image_width  = cfg.get_image_width();
    int const image_height = static_cast<int>(static_cast<double>(image_width) / aspect_ratio);
    auto const lookfrom    = cfg.get_camera_position();
    auto const lookat      = cfg.get_camera_target();
    auto const vup         = cfg.get_camera_north();
    double const vfov      = cfg.get_field_of_view();
    auto const focal_vector =
        lookfrom - lookat;  // Validar que la posición y el objetivo no sean iguales
    if (focal_vector.is_near_zero()) {
      throw std::runtime_error("Camera position and target cannot be the same");
    }
    double const focal_distance = focal_vector.magnitude();
    auto const w                = focal_vector.normalized();
    auto const theta            = degrees_to_radians(vfov);
    auto const h = std::tan(theta / 2.0);  // Calcular las dimensiones del viewport según FOV
    auto const viewport_height = 2.0 * h * focal_distance;
    auto const viewport_width  = aspect_ratio * viewport_height;
    auto const u_vec = vector::cross(vup, w);  // Construir la base ortonormal de la cámara
    if (u_vec.is_near_zero()) {
      throw std::runtime_error("Camera north vector cannot be parallel to view direction");
    }
    auto const u = u_vec.normalized();
    auto const v = vector::cross(w, u);
    horizontal   = viewport_width * u;  // Calcular los vectores del viewport
    vertical     = -viewport_height * v;
    double const inv_width =
        1.0 / static_cast<double>(image_width);  // Calcular offsets para el centrado de píxeles
    double const inv_height = 1.0 / static_cast<double>(image_height);
    auto const delta_u      = horizontal * inv_width;
    auto const delta_v      = vertical * inv_height;
    origin            = lookfrom;  // Posicionar origen y esquina inferior izquierda del viewport
    lower_left_corner = origin -
                        horizontal * 0.5 -
                        vertical * 0.5 -
                        focal_distance * w +
                        delta_u * 0.5 +
                        delta_v * 0.5;
  }

}  // namespace render
