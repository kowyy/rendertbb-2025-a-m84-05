#ifndef RENDER_RAY_HPP
#define RENDER_RAY_HPP

#include "vector.hpp"

namespace render {

  // Representa un rayo con origen y dirección para ray tracing
  class ray {
  public:
    ray() = default;

    // Constructor que valida que la dirección no sea cero
    ray(vector const & origin, vector const & direction) : orig{origin}, dir{direction} {
      if (dir.magnitude_squared() < epsilon) {
        throw std::invalid_argument(
            "La dirección del rayo no puede ser el vector cero o casi cero.");
      }
    }

    [[nodiscard]] vector get_origin() const { return orig; }

    [[nodiscard]] vector get_direction() const { return dir; }

    // Calcula el punto a lo largo del rayo en el tiempo
    [[nodiscard]] vector at(double t) const { return orig + dir * t; }

  private:
    vector orig;
    vector dir;
  };

}  // namespace render

#endif
