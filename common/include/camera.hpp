#ifndef RENDER_CAMERA_HPP
#define RENDER_CAMERA_HPP

#include "config.hpp"
#include "ray.hpp"
#include "vector.hpp"

namespace render {

  // Cámara virtual que genera rayos para el ray tracing
  class camera {
  public:
    explicit camera(config const & cfg);

    // Genera un rayo desde la cámara hacia las coordenadas (u,v)
    [[nodiscard]] ray get_ray(double u, double v) const {
      auto const direction = lower_left_corner + u * horizontal + v * vertical - origin;
      return ray{origin, direction};
    }

  private:
    vector origin;             // Posición de la cámara
    vector lower_left_corner;  // Esquina inferior izquierda del viewport
    vector horizontal;         // Vector horizontal del viewport
    vector vertical;           // Vector vertical del viewport
  };

}  // namespace render

#endif
