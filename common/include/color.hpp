#ifndef RENDER_COLOR_HPP
#define RENDER_COLOR_HPP

#include "vector.hpp"
#include <cstdint>

namespace render {

  // Representa un color RGB
  class color {
  public:
    // Constructores
    color() : rgb_{0.0, 0.0, 0.0} { }

    color(double r, double g, double b) : rgb_{r, g, b} { }

    explicit color(vector const & rgb) : rgb_{rgb} { }

    // Acceso a componentes RGB
    [[nodiscard]] double get_r() const { return rgb_.x; }

    [[nodiscard]] double get_g() const { return rgb_.y; }

    [[nodiscard]] double get_b() const { return rgb_.z; }

    // Conversión a valores discretos [0-255] con corrección gamma
    [[nodiscard]] std::uint8_t to_discrete_r(double gamma) const;
    [[nodiscard]] std::uint8_t to_discrete_g(double gamma) const;
    [[nodiscard]] std::uint8_t to_discrete_b(double gamma) const;

    // Operadores de modificación
    color & operator+=(color const & other) {
      rgb_ = rgb_ + other.rgb_;
      return *this;
    }

    color & operator*=(double scalar) {
      rgb_ = rgb_ * scalar;
      return *this;
    }

    // Multiplicación componente a componente
    color & operator*=(color const & other) {
      rgb_ = vector{rgb_.x * other.rgb_.x, rgb_.y * other.rgb_.y, rgb_.z * other.rgb_.z};
      return *this;
    }

    color & operator/=(double scalar) {
      rgb_ = rgb_ / scalar;
      return *this;
    }

    // Acceso directo al vector interno
    [[nodiscard]] vector const & as_vector() const { return rgb_; }

  private:
    vector rgb_;

    // Aplica corrección gamma
    static double apply_gamma_correction(double value, double gamma);

    // Convierte valor [0,1] a [0,255]
    static std::uint8_t to_discrete(double value);
  };

  // Operadores aritméticos
  inline color operator+(color const & lhs, color const & rhs) {
    return color{lhs.as_vector() + rhs.as_vector()};
  }

  inline color operator*(color const & lhs, double scalar) {
    return color{lhs.as_vector() * scalar};
  }

  inline color operator*(double scalar, color const & rhs) {
    return rhs * scalar;
  }

  // Modulación de color
  inline color operator*(color const & lhs, color const & rhs) {
    return color{lhs.get_r() * rhs.get_r(), lhs.get_g() * rhs.get_g(), lhs.get_b() * rhs.get_b()};
  }

  inline color operator/(color const & lhs, double scalar) {
    return color{lhs.as_vector() / scalar};
  }

}  // namespace render

#endif
