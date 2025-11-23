#include "vector.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <stdexcept>

namespace render {

  // Comprueba que el constructor por defecto inicializa todos los componentes (x, y, z) a cero.
  TEST(VectorTest, DefaultConstructorInitializesToZero) {
    vector const vec;
    EXPECT_DOUBLE_EQ(vec.x, 0.0);
    EXPECT_DOUBLE_EQ(vec.y, 0.0);
    EXPECT_DOUBLE_EQ(vec.z, 0.0);
  }

  // Comprueba que el constructor parametrizado inicializa correctamente los componentes.
  TEST(VectorTest, ParameterizedConstructorInitializesCorrectly) {
    vector const vec{1.5, 2.5, 3.5};
    EXPECT_DOUBLE_EQ(vec.x, 1.5);
    EXPECT_DOUBLE_EQ(vec.y, 2.5);
    EXPECT_DOUBLE_EQ(vec.z, 3.5);
  }

  // Comprueba que el constructor acepta valores negativos.
  TEST(VectorTest, ConstructorWithNegativeValues) {
    vector const vec{-1.0, -2.0, -3.0};
    EXPECT_DOUBLE_EQ(vec.x, -1.0);
    EXPECT_DOUBLE_EQ(vec.y, -2.0);
    EXPECT_DOUBLE_EQ(vec.z, -3.0);
  }

  // Tests de Magnitud

  // Comprueba que la magnitud de un vector cero es 0.0.
  TEST(VectorTest, MagnitudeOfZeroVector) {
    vector const vec{0.0, 0.0, 0.0};
    EXPECT_DOUBLE_EQ(vec.magnitude(), 0.0);
  }

  // Verifica la magnitud de un vector unitario en X es 1.0.
  TEST(VectorTest, MagnitudeOfUnitVectorX) {
    vector const vec{1.0, 0.0, 0.0};
    EXPECT_DOUBLE_EQ(vec.magnitude(), 1.0);
  }

  // Lo mismo pero en Y
  TEST(VectorTest, MagnitudeOfUnitVectorY) {
    vector const vec{0.0, 1.0, 0.0};
    EXPECT_DOUBLE_EQ(vec.magnitude(), 1.0);
  }

  // Lo mismo pero en Z
  TEST(VectorTest, MagnitudeOfUnitVectorZ) {
    vector const vec{0.0, 0.0, 1.0};
    EXPECT_DOUBLE_EQ(vec.magnitude(), 1.0);
  }

  // Comprueba la magnitud de un vector 2D
  TEST(VectorTest, MagnitudeOf345Triangle) {
    vector const vec{3.0, 4.0, 0.0};
    EXPECT_DOUBLE_EQ(vec.magnitude(), 5.0);
  }

  // Comprueba la magnitud de un vector 3D
  TEST(VectorTest, MagnitudeOf3DVector) {
    vector const vec{1.0, 2.0, 2.0};
    EXPECT_DOUBLE_EQ(vec.magnitude(), 3.0);
  }

  // Verifica la magnitud al cuadrado de un vector con componentes positivas
  TEST(VectorTest, MagnitudeSquaredAvoidsSqrt) {
    vector const vec{3.0, 4.0, 0.0};
    EXPECT_DOUBLE_EQ(vec.magnitude_squared(), 25.0);
  }

  // Verifica la magnitud al cuadrado de un vector con componentes negativas
  TEST(VectorTest, MagnitudeSquaredOfNegativeComponents) {
    vector const vec{-3.0, -4.0, 0.0};
    EXPECT_DOUBLE_EQ(vec.magnitude_squared(), 25.0);
  }

  // Tests de Normalización

  // Verifica que un vector normalizado tiene una magnitud cercana a 1.0
  TEST(VectorTest, NormalizedVectorHasMagnitudeOne) {
    vector const vec{3.0, 4.0, 0.0};
    vector const normalized = vec.normalized();
    EXPECT_NEAR(normalized.magnitude(), 1.0, 1e-10);
  }

  // Verifica que la normalización preserva la dirección del vector.
  TEST(VectorTest, NormalizedPreservesDirection) {
    vector const vec{2.0, 0.0, 0.0};
    vector const normalized = vec.normalized();
    EXPECT_DOUBLE_EQ(normalized.x, 1.0);
    EXPECT_DOUBLE_EQ(normalized.y, 0.0);
    EXPECT_DOUBLE_EQ(normalized.z, 0.0);
  }

  // Comprueba los componentes de un vector normalizado
  TEST(VectorTest, NormalizedOf345Vector) {
    vector const vec{3.0, 4.0, 0.0};
    vector const normalized = vec.normalized();
    EXPECT_DOUBLE_EQ(normalized.x, 0.6);
    EXPECT_DOUBLE_EQ(normalized.y, 0.8);
    EXPECT_DOUBLE_EQ(normalized.z, 0.0);
  }

  // Verifica que normalizar un vector cero lanza una excepción.
  TEST(VectorTest, NormalizedThrowsOnZeroVector) {
    vector const vec{0.0, 0.0, 0.0};
    EXPECT_THROW({ [[maybe_unused]] auto const result = vec.normalized(); }, std::runtime_error);
  }

  // Verifica que normalizar un vector muy pequeño lanza una excepción.
  TEST(VectorTest, NormalizedThrowsOnNearZeroVector) {
    vector const vec{1e-10, 1e-10, 1e-10};
    EXPECT_THROW({ [[maybe_unused]] auto const result = vec.normalized(); }, std::runtime_error);
  }

  // Tests de Producto Escalar

  // Verifica el producto escalar de dos vectores ortogonales es cero.
  TEST(VectorTest, DotProductOfOrthogonalVectors) {
    vector const v1{1.0, 0.0, 0.0};
    vector const v2{0.0, 1.0, 0.0};
    EXPECT_DOUBLE_EQ(vector::dot(v1, v2), 0.0);
  }

  // Verifica el producto escalar de dos vectores paralelos.
  TEST(VectorTest, DotProductOfParallelVectors) {
    vector const v1{1.0, 0.0, 0.0};
    vector const v2{2.0, 0.0, 0.0};
    EXPECT_DOUBLE_EQ(vector::dot(v1, v2), 2.0);
  }

  // Prueba el producto escalar de dos vectores opuestos.
  TEST(VectorTest, DotProductOfOppositeVectors) {
    vector const v1{1.0, 0.0, 0.0};
    vector const v2{-1.0, 0.0, 0.0};
    EXPECT_DOUBLE_EQ(vector::dot(v1, v2), -1.0);
  }

  // Verifica que el producto escalar es conmutativo
  TEST(VectorTest, DotProductCommutative) {
    vector const v1{1.0, 2.0, 3.0};
    vector const v2{4.0, 5.0, 6.0};
    EXPECT_DOUBLE_EQ(vector::dot(v1, v2), vector::dot(v2, v1));
  }

  // Comprueba el producto escalar en un caso general 3D
  TEST(VectorTest, DotProductGeneralCase) {
    vector const v1{1.0, 2.0, 3.0};
    vector const v2{4.0, 5.0, 6.0};
    EXPECT_DOUBLE_EQ(vector::dot(v1, v2), 32.0);
  }

  // Tests de Producto Vectorial

  // Verifica el producto vectorial de dos vectores unitarios.
  TEST(VectorTest, CrossProductOfUnitVectors) {
    vector const v1{1.0, 0.0, 0.0};
    vector const v2{0.0, 1.0, 0.0};
    vector const result = vector::cross(v1, v2);
    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 1.0);
  }

  // Verifica que el producto vectorial es anticomutativo
  TEST(VectorTest, CrossProductAntiCommutative) {
    vector const v1{1.0, 2.0, 3.0};
    vector const v2{4.0, 5.0, 6.0};
    vector const cross12 = vector::cross(v1, v2);
    vector const cross21 = vector::cross(v2, v1);
    EXPECT_DOUBLE_EQ(cross12.x, -cross21.x);
    EXPECT_DOUBLE_EQ(cross12.y, -cross21.y);
    EXPECT_DOUBLE_EQ(cross12.z, -cross21.z);
  }

  // Verifica que el producto vectorial de vectores paralelos es el vector cero.
  TEST(VectorTest, CrossProductOfParallelVectorsIsZero) {
    vector const v1{1.0, 2.0, 3.0};
    vector const v2{2.0, 4.0, 6.0};
    vector const result = vector::cross(v1, v2);
    EXPECT_NEAR(result.magnitude(), 0.0, 1e-10);
  }

  // Verifica que el producto vectorial es perpendicular a los vectores de entrada.
  TEST(VectorTest, CrossProductPerpendicularToInputs) {
    vector const v1{1.0, 0.0, 0.0};
    vector const v2{0.0, 1.0, 0.0};
    vector const result = vector::cross(v1, v2);
    EXPECT_DOUBLE_EQ(vector::dot(result, v1), 0.0);
    EXPECT_DOUBLE_EQ(vector::dot(result, v2), 0.0);
  }

  // Tests de Operadores Aritméticos

  // Verifica la suma de dos vectores.
  TEST(VectorTest, AdditionOfVectors) {
    vector const v1{1.0, 2.0, 3.0};
    vector const v2{4.0, 5.0, 6.0};
    vector const result = v1 + v2;
    EXPECT_DOUBLE_EQ(result.x, 5.0);
    EXPECT_DOUBLE_EQ(result.y, 7.0);
    EXPECT_DOUBLE_EQ(result.z, 9.0);
  }

  // Comprueba la resta de dos vectores.
  TEST(VectorTest, SubtractionOfVectors) {
    vector const v1{4.0, 5.0, 6.0};
    vector const v2{1.0, 2.0, 3.0};
    vector const result = v1 - v2;
    EXPECT_DOUBLE_EQ(result.x, 3.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
  }

  // Verifica la multiplicación por un escalar.
  TEST(VectorTest, ScalarMultiplication) {
    vector const vec{1.0, 2.0, 3.0};
    vector const result = vec * 2.0;
    EXPECT_DOUBLE_EQ(result.x, 2.0);
    EXPECT_DOUBLE_EQ(result.y, 4.0);
    EXPECT_DOUBLE_EQ(result.z, 6.0);
  }

  // Verifica que la multiplicación por escalar es conmutativa
  TEST(VectorTest, ScalarMultiplicationCommutative) {
    vector const vec{1.0, 2.0, 3.0};
    vector const result1 = vec * 2.0;
    vector const result2 = 2.0 * vec;
    EXPECT_DOUBLE_EQ(result1.x, result2.x);
    EXPECT_DOUBLE_EQ(result1.y, result2.y);
    EXPECT_DOUBLE_EQ(result1.z, result2.z);
  }

  // Verifica la división por un escalar.
  TEST(VectorTest, ScalarDivision) {
    vector const vec{2.0, 4.0, 6.0};
    vector const result = vec / 2.0;
    EXPECT_DOUBLE_EQ(result.x, 1.0);
    EXPECT_DOUBLE_EQ(result.y, 2.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
  }

  // Verifica que la división por cero lanza una excepción.
  TEST(VectorTest, DivisionByZeroThrows) {
    vector const vec{1.0, 2.0, 3.0};
    EXPECT_THROW(vec / 0.0, std::runtime_error);
  }

  // Verifica que la división por un escalar muy cercano a cero lanza una excepción.
  TEST(VectorTest, DivisionByNearZeroThrows) {
    vector const vec{1.0, 2.0, 3.0};
    EXPECT_THROW(vec / 1e-10, std::runtime_error);
  }

  // Verifica el operador de negación unaria.
  TEST(VectorTest, UnaryNegation) {
    vector const vec{1.0, -2.0, 3.0};
    vector const result = -vec;
    EXPECT_DOUBLE_EQ(result.x, -1.0);
    EXPECT_DOUBLE_EQ(result.y, 2.0);
    EXPECT_DOUBLE_EQ(result.z, -3.0);
  }

  // Verifica que la doble negación devuelve el vector original.
  TEST(VectorTest, DoubleNegationReturnsOriginal) {
    vector const vec{1.0, 2.0, 3.0};
    vector const result = -(-vec);
    EXPECT_DOUBLE_EQ(result.x, vec.x);
    EXPECT_DOUBLE_EQ(result.y, vec.y);
    EXPECT_DOUBLE_EQ(result.z, vec.z);
  }

  // Tests de is_near_zero
  // Verifica que is_near_zero devuelve true para el vector cero
  TEST(VectorTest, IsNearZeroForZeroVector) {
    vector const vec{0.0, 0.0, 0.0};
    EXPECT_TRUE(vec.is_near_zero());
  }

  // Verifica que is_near_zero devuelve true para un vector con componentes muy pequeñas
  TEST(VectorTest, IsNearZeroForVerySmallVector) {
    vector const vec{1e-9, 1e-9, 1e-9};
    EXPECT_TRUE(vec.is_near_zero());
  }

  // Verifica que is_near_zero devuelve false para un vector normal
  TEST(VectorTest, IsNotNearZeroForNormalVector) {
    vector const vec{0.1, 0.0, 0.0};
    EXPECT_FALSE(vec.is_near_zero());
  }

  // Comprueba que is_near_zero devuelve false para un vector unitario
  TEST(VectorTest, IsNotNearZeroForUnitVector) {
    vector const vec{1.0, 0.0, 0.0};
    EXPECT_FALSE(vec.is_near_zero());
  }

  // Tests de perpendicular_to
  // Verifica que el vector perpendicular se calcula correctamente.
  TEST(VectorTest, PerpendicularToAxis) {
    vector const vec{1.0, 1.0, 0.0};
    vector const axis{1.0, 0.0, 0.0};
    vector const perp = vec.perpendicular_to(axis);
    EXPECT_DOUBLE_EQ(perp.x, 0.0);
    EXPECT_DOUBLE_EQ(perp.y, 1.0);
    EXPECT_DOUBLE_EQ(perp.z, 0.0);
  }

  // Verifica que el vector perpendicular es efectivamente ortogonal al eje dado.
  TEST(VectorTest, PerpendicularToAxisOrthogonal) {
    vector const vec{1.0, 1.0, 0.0};
    vector const axis{1.0, 0.0, 0.0};
    vector const perp = vec.perpendicular_to(axis);
    EXPECT_NEAR(vector::dot(perp, axis), 0.0, 1e-10);
  }

  // Comprueba que el vector perpendicular de un vector paralelo al eje es el vector cero.
  TEST(VectorTest, PerpendicularToParallelVectorIsZero) {
    vector const vec{2.0, 0.0, 0.0};
    vector const axis{1.0, 0.0, 0.0};
    vector const perp = vec.perpendicular_to(axis);
    EXPECT_TRUE(perp.is_near_zero());
  }

  // Verifica el cálculo del vector perpendicular en un caso 3D general.
  TEST(VectorTest, PerpendicularTo3DCase) {
    vector const vec{1.0, 2.0, 3.0};
    vector const axis{0.0, 0.0, 1.0};
    vector const perp = vec.perpendicular_to(axis);
    EXPECT_DOUBLE_EQ(perp.x, 1.0);
    EXPECT_DOUBLE_EQ(perp.y, 2.0);
    EXPECT_DOUBLE_EQ(perp.z, 0.0);
  }

  // Tests de Getters
  // Verifica que los getters devuelven los valores correctos.
  TEST(VectorTest, GettersReturnCorrectValues) {
    vector const vec{1.5, 2.5, 3.5};
    EXPECT_DOUBLE_EQ(vec.x, 1.5);
    EXPECT_DOUBLE_EQ(vec.y, 2.5);
    EXPECT_DOUBLE_EQ(vec.z, 3.5);
  }

  // Comprueba la multiplicación por un escalar con valor negativo.
  TEST(VectorTest, OperationsWithNegativeScalars) {
    vector const vec{1.0, 2.0, 3.0};
    vector const result = vec * -1.0;
    EXPECT_DOUBLE_EQ(result.x, -1.0);
    EXPECT_DOUBLE_EQ(result.y, -2.0);
    EXPECT_DOUBLE_EQ(result.z, -3.0);
  }

  // Comprueba que sumar un vector cero no altera el vector original.
  TEST(VectorTest, AddingZeroVectorDoesNotChange) {
    vector const vec{1.0, 2.0, 3.0};
    vector const zero{0.0, 0.0, 0.0};
    vector const result = vec + zero;
    EXPECT_DOUBLE_EQ(result.x, vec.x);
    EXPECT_DOUBLE_EQ(result.y, vec.y);
    EXPECT_DOUBLE_EQ(result.z, vec.z);
  }

  // Comprueba que restar un vector a sí mismo da el vector cero.
  TEST(VectorTest, SubtractingVectorFromItselfGivesZero) {
    vector const vec1{1.0, 2.0, 3.0};
    vector const vec2{1.0, 2.0, 3.0};
    vector const result = vec1 - vec2;
    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 0.0);
  }

  // Comprueba que multiplicar por uno no altera el vector original.
  TEST(VectorTest, MultiplyingByOnePreservesVector) {
    vector const vec{1.0, 2.0, 3.0};
    vector const result = vec * 1.0;
    EXPECT_DOUBLE_EQ(result.x, vec.x);
    EXPECT_DOUBLE_EQ(result.y, vec.y);
    EXPECT_DOUBLE_EQ(result.z, vec.z);
  }

}  // namespace render
