#include "color.hpp"
#include "vector.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace render {

  // Convierte componente R a formato discreto con corrección gamma
  std::uint8_t color::to_discrete_r(double const gamma) const {
    double const clamped_linear = std::clamp(rgb_.x, 0.0, 1.0);
    double const corrected      = apply_gamma_correction(clamped_linear, gamma);
    return to_discrete(corrected);
  }

  // Convierte componente G a formato discreto con corrección gamma
  std::uint8_t color::to_discrete_g(double const gamma) const {
    double const clamped_linear = std::clamp(rgb_.y, 0.0, 1.0);
    double const corrected      = apply_gamma_correction(clamped_linear, gamma);
    return to_discrete(corrected);
  }

  // Convierte componente B a formato discreto con corrección gamma
  std::uint8_t color::to_discrete_b(double const gamma) const {
    double const clamped_linear = std::clamp(rgb_.z, 0.0, 1.0);
    double const corrected      = apply_gamma_correction(clamped_linear, gamma);
    return to_discrete(corrected);
  }

  // Aplica corrección gamma transformando el espacio lineal a espacio RGB
  double color::apply_gamma_correction(double const value, double const gamma) {
    if (value < 0.0) {
      return 0.0;
    }
    return std::pow(value, 1.0 / gamma);
  }

  // Escala valor [0,1] a rango [0,255] para formato de imagen
  std::uint8_t color::to_discrete(double const value) {
    return static_cast<std::uint8_t>(value * 255.0);
  }

}  // namespace render
