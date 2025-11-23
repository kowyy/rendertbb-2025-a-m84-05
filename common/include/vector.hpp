#ifndef RENDER_VECTOR_HPP
#define RENDER_VECTOR_HPP

#include <cmath>
#include <ostream>

namespace render {

  // Tolerancia para comparaciones de precisión numérica
  constexpr double epsilon = 1e-8;

  // Vector 3D para cálculos
  class vector {
  public:
    double x, y, z;

    // Constructores
    vector() : x{0.0}, y{0.0}, z{0.0} { }

    vector(double cx, double cy, double cz) : x{cx}, y{cy}, z{cz} { }

    // Magnitud del vector
    [[nodiscard]] double magnitude() const { return std::sqrt(x * x + y * y + z * z); }

    [[nodiscard]] double magnitude_squared() const { return x * x + y * y + z * z; }

    // Devuelve el vector unitario en la misma dirección
    [[nodiscard]] vector normalized() const {
      double const mag = magnitude();
      if (mag < epsilon) {
        throw std::runtime_error("Intento de normalizar un vector cero o casi cero.");
      }
      double const inv_mag = 1.0 / mag;
      return vector{x * inv_mag, y * inv_mag, z * inv_mag};
    }

    // Producto escalar entre dos vectores
    [[nodiscard]] static double dot(vector const & a, vector const & b) {
      return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // Producto vectorial entre dos vectores
    [[nodiscard]] static vector cross(vector const & a, vector const & b) {
      return vector{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    // Comprueba si el vector es casi cero en todas sus componentes
    [[nodiscard]] bool is_near_zero() const {
      return (x > -epsilon and x < epsilon) and
             (y > -epsilon and y < epsilon) and
             (z > -epsilon and z < epsilon);
    }

    // Calcula la componente perpendicular a un eje dado
    [[nodiscard]] vector perpendicular_to(vector const & axis) const {
      double const parallel_component = dot(*this, axis);
      return *this - axis * parallel_component;
    }

    // Operadores aritméticos
    vector operator+(vector const & other) const {
      return vector{x + other.x, y + other.y, z + other.z};
    }

    vector operator-(vector const & other) const {
      return vector{x - other.x, y - other.y, z - other.z};
    }

    vector operator*(double scalar) const { return vector{x * scalar, y * scalar, z * scalar}; }

    vector operator/(double scalar) const {
      if (std::abs(scalar) < epsilon) {
        throw std::runtime_error("Vector division by zero or near-zero scalar.");
      }
      double const inv_scalar = 1.0 / scalar;
      return vector{x * inv_scalar, y * inv_scalar, z * inv_scalar};
    }

    vector operator-() const { return vector{-x, -y, -z}; }

    friend std::ostream & operator<<(std::ostream & os, vector const & vec);
  };

  // Permite multiplicación scalar * vector
  inline vector operator*(double scalar, vector const & v) {
    return v * scalar;
  }

  // Operador de salida para depuración
  inline std::ostream & operator<<(std::ostream & os, vector const & v) {
    os << "vector(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
  }

}  // namespace render

#endif
