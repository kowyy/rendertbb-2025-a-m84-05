#include "ray.hpp"
#include "vector.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

namespace render {

  // Comprueba que el constructor inicializa correctamente el origen y la dirección del rayo.
  TEST(RayTest, ConstructorInitializesOriginAndDirection) {
    vector const origin{1.0, 2.0, 3.0};
    vector const direction{0.0, 1.0, 0.0};
    ray const r{origin, direction};
    vector const result_origin = r.get_origin();
    EXPECT_DOUBLE_EQ(result_origin.x, 1.0);
    EXPECT_DOUBLE_EQ(result_origin.y, 2.0);
    EXPECT_DOUBLE_EQ(result_origin.z, 3.0);
    vector const result_direction = r.get_direction();
    EXPECT_DOUBLE_EQ(result_direction.x, 0.0);
    EXPECT_DOUBLE_EQ(result_direction.y, 1.0);
    EXPECT_DOUBLE_EQ(result_direction.z, 0.0);
  }

  // Comprueba que el método 'at' devuelve el punto correcto para un valor de t dado.
  TEST(RayTest, AtReturnsCorrectPoint) {
    vector const origin{0.0, 0.0, 0.0};
    vector const direction{1.0, 0.0, 0.0};
    ray const r{origin, direction};
    vector const point = r.at(5.0);
    EXPECT_DOUBLE_EQ(point.x, 5.0);
    EXPECT_DOUBLE_EQ(point.y, 0.0);
    EXPECT_DOUBLE_EQ(point.z, 0.0);
  }

  // Comprueba que el método 'at' funciona con valores negativos de t.
  TEST(RayTest, AtWithNegativeT) {
    vector const origin{10.0, 5.0, 0.0};
    vector const direction{-1.0, 0.0, 0.0};
    ray const r{origin, direction};
    vector const point = r.at(3.0);
    EXPECT_DOUBLE_EQ(point.x, 7.0);
    EXPECT_DOUBLE_EQ(point.y, 5.0);
    EXPECT_DOUBLE_EQ(point.z, 0.0);
  }

  // Comprueba que el método 'at' funciona con t igual a cero.
  TEST(RayTest, AtWithZeroT) {
    vector const origin{1.0, 2.0, 3.0};
    vector const direction{4.0, 5.0, 6.0};
    ray const r{origin, direction};
    vector const point = r.at(0.0);
    EXPECT_DOUBLE_EQ(point.x, 1.0);
    EXPECT_DOUBLE_EQ(point.y, 2.0);
    EXPECT_DOUBLE_EQ(point.z, 3.0);
  }

  // Verifica que el método 'at' funciona con una dirección diagonal.
  TEST(RayTest, AtWithDiagonalDirection) {
    vector const origin{0.0, 0.0, 0.0};
    vector const direction{1.0, 1.0, 1.0};
    ray const r{origin, direction};
    vector const point = r.at(2.0);
    EXPECT_DOUBLE_EQ(point.x, 2.0);
    EXPECT_DOUBLE_EQ(point.y, 2.0);
    EXPECT_DOUBLE_EQ(point.z, 2.0);
  }

  // Comprueba que el constructor lanza una excepción si la dirección es el vector cero.
  TEST(RayTest, ZeroDirectionThrowsException) {
    vector const origin{0.0, 0.0, 0.0};
    vector const zero_direction{0.0, 0.0, 0.0};

    EXPECT_THROW(ray(origin, zero_direction), std::invalid_argument);
  }

  // Comprueba que el constructor por defecto inicializa los vectores de origen y dirección a cero.
  TEST(RayTest, DefaultConstructorInitializesToZero) {
    render::ray const r;
    render::vector const origin = r.get_origin();
    render::vector const dir    = r.get_direction();

    EXPECT_DOUBLE_EQ(origin.x, 0.0);
    EXPECT_DOUBLE_EQ(origin.y, 0.0);
    EXPECT_DOUBLE_EQ(origin.z, 0.0);
    EXPECT_DOUBLE_EQ(dir.x, 0.0);
    EXPECT_DOUBLE_EQ(dir.y, 0.0);
    EXPECT_DOUBLE_EQ(dir.z, 0.0);
  }

}  // namespace render
