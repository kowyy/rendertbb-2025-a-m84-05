#include "color.hpp"
#include "vector.hpp"
#include <cmath>
#include <cstdint>
#include <gtest/gtest.h>

namespace render {

  // Comprueba que el constructor por defecto inicializa el color a negro (0, 0, 0).
  TEST(ColorTest, DefaultConstructor) {
    color const c;
    EXPECT_DOUBLE_EQ(c.get_r(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.0);
  }

  // Verifica que el constructor que acepta R, G, B asigna los valores correctamente.
  TEST(ColorTest, ConstructorWithValidValues) {
    color const c(0.3, 0.6, 0.9);
    EXPECT_DOUBLE_EQ(c.get_r(), 0.3);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.6);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.9);
  }

  // Verifica que el constructor permite valores negativos.
  TEST(ColorTest, ConstructorAllowsNegativeValues) {
    color const c(-0.5, -1.0, -0.1);
    EXPECT_DOUBLE_EQ(c.get_r(), -0.5);
    EXPECT_DOUBLE_EQ(c.get_g(), -1.0);
    EXPECT_DOUBLE_EQ(c.get_b(), -0.1);
  }

  // Comprueba que el constructor permite valores por encima de uno.
  TEST(ColorTest, ConstructorAllowsValuesAboveOne) {
    color const c(1.5, 2.0, 1.1);
    EXPECT_DOUBLE_EQ(c.get_r(), 1.5);
    EXPECT_DOUBLE_EQ(c.get_g(), 2.0);
    EXPECT_DOUBLE_EQ(c.get_b(), 1.1);
  }

  // Verifica que el constructor permite una mezcla de valores negativos y por encima de uno.
  TEST(ColorTest, ConstructorAllowsMixedValues) {
    color const c(-0.5, 0.5, 1.5);
    EXPECT_DOUBLE_EQ(c.get_r(), -0.5);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.5);
    EXPECT_DOUBLE_EQ(c.get_b(), 1.5);
  }

  // Prueba el constructor con valores límite comunes
  TEST(ColorTest, ConstructorWithBoundaryValues) {
    color const c(0.0, 1.0, 0.5);
    EXPECT_DOUBLE_EQ(c.get_r(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_g(), 1.0);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.5);
  }

  TEST(ColorTest, ConstructorFromVector) {
    vector const v(0.2, 0.5, 0.8);
    color const c(v);
    EXPECT_DOUBLE_EQ(c.get_r(), 0.2);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.5);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.8);
  }

  // Verifica que el constructor desde vector maneja valores fuera del rango [0, 1]
  TEST(ColorTest, ConstructorFromVectorAllowsValues) {
    vector const v(-0.3, 1.5, 0.5);
    color const c(v);
    EXPECT_DOUBLE_EQ(c.get_r(), -0.3);
    EXPECT_DOUBLE_EQ(c.get_g(), 1.5);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.5);
  }

  // Tests de Conversión a Valores Discretos
  TEST(ColorTest, ToDiscreteWithGammaOne) {
    color const c(0.0, 0.5, 1.0);
    EXPECT_EQ(c.to_discrete_r(1.0), 0);
    EXPECT_EQ(c.to_discrete_g(1.0), 127);
    EXPECT_EQ(c.to_discrete_b(1.0), 255);
  }

  // Verifica que los valores fuera del rango [0, 1] se aproximan correctamente
  TEST(ColorTest, ToDiscreteClampsValues) {
    color const c(-0.5, 1.5, 1.0);
    EXPECT_EQ(c.to_discrete_r(1.0), 0);
    EXPECT_EQ(c.to_discrete_g(1.0), 255);
  }

  // Prueba con colores extremos
  TEST(ColorTest, ToDiscreteBlack) {
    color const c(0.0, 0.0, 0.0);
    EXPECT_EQ(c.to_discrete_r(2.2), 0);
    EXPECT_EQ(c.to_discrete_g(2.2), 0);
    EXPECT_EQ(c.to_discrete_b(2.2), 0);
  }

  // Prueba con colores blancos
  TEST(ColorTest, ToDiscreteWhite) {
    color const c(1.0, 1.0, 1.0);
    EXPECT_EQ(c.to_discrete_r(2.2), 255);
    EXPECT_EQ(c.to_discrete_g(2.2), 255);
    EXPECT_EQ(c.to_discrete_b(2.2), 255);
  }

  // Prueba con corrección gamma estándar
  TEST(ColorTest, GammaCorrectionStandard) {
    color const c(0.5, 0.5, 0.5);
    double const gamma  = 2.2;
    auto const expected = static_cast<std::uint8_t>(std::pow(0.5, 1.0 / gamma) * 255.0);
    EXPECT_EQ(c.to_discrete_r(gamma), expected);
    EXPECT_EQ(c.to_discrete_g(gamma), expected);
    EXPECT_EQ(c.to_discrete_b(gamma), expected);
  }

  // Prueba con corrección gamma diferente
  TEST(ColorTest, GammaCorrectionDifferentValues) {
    color const c(0.25, 0.5, 0.75);
    double const gamma    = 2.2;
    auto const expected_r = static_cast<std::uint8_t>(std::pow(0.25, 1.0 / gamma) * 255.0);
    auto const expected_g = static_cast<std::uint8_t>(std::pow(0.5, 1.0 / gamma) * 255.0);
    auto const expected_b = static_cast<std::uint8_t>(std::pow(0.75, 1.0 / gamma) * 255.0);
    EXPECT_EQ(c.to_discrete_r(gamma), expected_r);
    EXPECT_EQ(c.to_discrete_g(gamma), expected_g);
    EXPECT_EQ(c.to_discrete_b(gamma), expected_b);
  }

  // Prueba la corrección gamma con un valor de gamma muy pequeño
  TEST(ColorTest, GammaCorrectionVerySmallGamma) {
    color const c(0.5, 0.5, 0.5);
    double const gamma = 0.5;
    EXPECT_NEAR(c.to_discrete_r(gamma), 63, 1);
  }

  // Igual que el anterior test pero con un valor de gamma muy grande
  TEST(ColorTest, GammaCorrectionVeryLargeGamma) {
    color const c(0.5, 0.5, 0.5);
    double const gamma = 5.0;
    EXPECT_NEAR(c.to_discrete_r(gamma), 222, 1);
  }

  // Verifica el operador += entre dos colores.
  TEST(ColorTest, AdditionAssignment) {
    color c(0.2, 0.3, 0.4);
    color const other(0.1, 0.2, 0.3);
    c += other;
    EXPECT_NEAR(c.get_r(), 0.3, 1e-9);
    EXPECT_NEAR(c.get_g(), 0.5, 1e-9);
    EXPECT_NEAR(c.get_b(), 0.7, 1e-9);
  }

  // Verifica que la suma y asignación acumule correctamente
  TEST(ColorTest, AdditionAssignmentAccumulates) {
    color c(0.8, 0.5, 0.3);
    color const other(0.5, 0.7, 0.9);
    c += other;
    EXPECT_NEAR(c.get_r(), 1.3, 1e-9);  // 0.8 + 0.5 = 1.3
    EXPECT_NEAR(c.get_g(), 1.2, 1e-9);  // 0.5 + 0.7 = 1.2
    EXPECT_NEAR(c.get_b(), 1.2, 1e-9);  // 0.3 + 0.9 = 1.2
  }

  // Verifica la suma y asignación con el color negro
  TEST(ColorTest, AdditionAssignmentWithBlack) {
    color c(0.5, 0.5, 0.5);
    color const black(0.0, 0.0, 0.0);
    c += black;
    EXPECT_DOUBLE_EQ(c.get_r(), 0.5);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.5);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.5);
  }

  // Verifica el operador *= con un escalar.
  TEST(ColorTest, ScalarMultiplicationAssignment) {
    color c(0.4, 0.6, 0.8);
    c *= 0.5;
    EXPECT_NEAR(c.get_r(), 0.2, 1e-9);
    EXPECT_NEAR(c.get_g(), 0.3, 1e-9);
    EXPECT_NEAR(c.get_b(), 0.4, 1e-9);
  }

  // Comprueba que la multiplicación por escalar acumula valores sobre 1.0
  TEST(ColorTest, ScalarMultiplicationAssignmentAccumulates) {
    color c(0.5, 0.7, 0.9);
    c *= 2.0;
    EXPECT_NEAR(c.get_r(), 1.0, 1e-9);
    EXPECT_NEAR(c.get_g(), 1.4, 1e-9);
    EXPECT_NEAR(c.get_b(), 1.8, 1e-9);
  }

  // Verifica la multiplicación por escalar con un valor de cero
  TEST(ColorTest, ScalarMultiplicationByZero) {
    color c(0.5, 0.6, 0.7);
    c *= 0.0;
    EXPECT_DOUBLE_EQ(c.get_r(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.0);
  }

  // Verifica el operador *= entre dos colores.
  TEST(ColorTest, ColorMultiplicationAssignment) {
    color c(0.5, 0.6, 0.8);
    color const other(0.4, 0.5, 0.25);
    c *= other;
    EXPECT_NEAR(c.get_r(), 0.2, 1e-9);  // 0.5 * 0.4
    EXPECT_NEAR(c.get_g(), 0.3, 1e-9);  // 0.6 * 0.5
    EXPECT_NEAR(c.get_b(), 0.2, 1e-9);  // 0.8 * 0.25
  }

  // Comprueba que multiplicar un color por blanco (1,1,1) no lo altera.
  TEST(ColorTest, ColorMultiplicationAssignmentWithWhite) {
    color c(0.3, 0.5, 0.7);
    color const white(1.0, 1.0, 1.0);
    c *= white;
    EXPECT_DOUBLE_EQ(c.get_r(), 0.3);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.5);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.7);
  }

  // Comprueba que multiplicar un color por negro (0,0,0) resulta en negro.
  TEST(ColorTest, ColorMultiplicationAssignmentWithBlack) {
    color c(0.5, 0.6, 0.7);
    color const black(0.0, 0.0, 0.0);
    c *= black;
    EXPECT_DOUBLE_EQ(c.get_r(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_g(), 0.0);
    EXPECT_DOUBLE_EQ(c.get_b(), 0.0);
  }

  // Verifica el operador de división por escalar
  TEST(ColorTest, DivisionAssignment) {
    color c(0.8, 0.6, 0.4);
    c /= 2.0;
    EXPECT_NEAR(c.get_r(), 0.4, 1e-9);
    EXPECT_NEAR(c.get_g(), 0.3, 1e-9);
    EXPECT_NEAR(c.get_b(), 0.2, 1e-9);
  }

  // Prueba la división por escalar para promediar
  TEST(ColorTest, DivisionAssignmentForAveraging) {
    color c(2.5, 3.0, 4.0);
    c /= 5.0;
    EXPECT_NEAR(c.get_r(), 0.5, 1e-9);  // 2.5 / 5.0
    EXPECT_NEAR(c.get_g(), 0.6, 1e-9);  // 3.0 / 5.0
    EXPECT_NEAR(c.get_b(), 0.8, 1e-9);  // 4.0 / 5.0
  }

  // Verifica el operador + entre dos colores
  TEST(ColorTest, AdditionOperator) {
    color const c1(0.2, 0.3, 0.4);
    color const c2(0.3, 0.4, 0.5);
    color const result = c1 + c2;

    EXPECT_NEAR(result.get_r(), 0.5, 1e-9);
    EXPECT_NEAR(result.get_g(), 0.7, 1e-9);
    EXPECT_NEAR(result.get_b(), 0.9, 1e-9);
  }

  // Comprueba que el operador + acumula valores sobre 1.0.
  TEST(ColorTest, AdditionOperatorAccumulates) {
    color const c1(0.7, 0.8, 0.9);
    color const c2(0.5, 0.6, 0.7);
    color const result = c1 + c2;
    EXPECT_NEAR(result.get_r(), 1.2, 1e-9);
    EXPECT_NEAR(result.get_g(), 1.4, 1e-9);
    EXPECT_NEAR(result.get_b(), 1.6, 1e-9);
  }

  // Verifica el operador * con un escalar.
  TEST(ColorTest, ScalarMultiplicationOperator) {
    color const c(0.4, 0.6, 0.8);
    color const result = c * 0.5;
    EXPECT_NEAR(result.get_r(), 0.2, 1e-9);
    EXPECT_NEAR(result.get_g(), 0.3, 1e-9);
    EXPECT_NEAR(result.get_b(), 0.4, 1e-9);
  }

  // Comprueba que la multiplicación por escalar es conmutativa
  TEST(ColorTest, ScalarMultiplicationOperatorCommutative) {
    color const c(0.4, 0.6, 0.8);
    color const result1 = c * 2.0;
    color const result2 = 2.0 * c;
    EXPECT_DOUBLE_EQ(result1.get_r(), result2.get_r());
    EXPECT_DOUBLE_EQ(result1.get_g(), result2.get_g());
    EXPECT_DOUBLE_EQ(result1.get_b(), result2.get_b());
  }

  // Verifica el operador * entre dos colores.
  TEST(ColorTest, ColorMultiplicationOperator) {
    color const c1(0.5, 0.6, 0.8);
    color const c2(0.4, 0.5, 0.25);
    color const result = c1 * c2;
    EXPECT_NEAR(result.get_r(), 0.2, 1e-9);
    EXPECT_NEAR(result.get_g(), 0.3, 1e-9);
    EXPECT_NEAR(result.get_b(), 0.2, 1e-9);
  }

  // Verifica el operador / con un escalar.
  TEST(ColorTest, DivisionOperator) {
    color const c(0.8, 0.6, 0.4);
    color const result = c / 2.0;
    EXPECT_NEAR(result.get_r(), 0.4, 1e-9);
    EXPECT_NEAR(result.get_g(), 0.3, 1e-9);
    EXPECT_NEAR(result.get_b(), 0.2, 1e-9);
  }

  // Prueba una cadena de operaciones de asignación
  TEST(ColorTest, ChainedOperations) {
    color c(0.5, 0.5, 0.5);
    c += color(0.2, 0.2, 0.2);
    c *= 0.5;
    c /= 2.0;
    EXPECT_NEAR(c.get_r(), 0.175, 1e-9);
    EXPECT_NEAR(c.get_g(), 0.175, 1e-9);
    EXPECT_NEAR(c.get_b(), 0.175, 1e-9);
  }

  // Prueba la acumulación y el promedio
  TEST(ColorTest, AccumulateAndAverage) {
    color accumulated(0.0, 0.0, 0.0);
    accumulated += color(0.2, 0.3, 0.4);
    accumulated += color(0.3, 0.4, 0.5);
    accumulated += color(0.4, 0.5, 0.6);
    accumulated += color(0.5, 0.6, 0.7);
    EXPECT_NEAR(accumulated.get_r(), 1.4, 1e-9);
    EXPECT_NEAR(accumulated.get_g(), 1.8, 1e-9);
    EXPECT_NEAR(accumulated.get_b(), 2.2, 1e-9);
    color const averaged = accumulated / 4.0;
    EXPECT_NEAR(averaged.get_r(), 0.35, 1e-9);
    EXPECT_NEAR(averaged.get_g(), 0.45, 1e-9);
    EXPECT_NEAR(averaged.get_b(), 0.55, 1e-9);
  }

  // Verifica la aplicación de reflectancia en un color.
  TEST(ColorTest, ReflectanceApplication) {
    color const light(0.8, 0.9, 1.0);
    color const reflectance(0.5, 0.6, 0.7);
    color const reflected = light * reflectance;
    EXPECT_NEAR(reflected.get_r(), 0.4, 1e-9);
    EXPECT_NEAR(reflected.get_g(), 0.54, 1e-9);
    EXPECT_NEAR(reflected.get_b(), 0.7, 1e-9);
  }

  // Prueba con valores muy pequeños cercanos a cero
  TEST(ColorTest, VerySmallValues) {
    color const c(1e-10, 1e-9, 1e-8);
    EXPECT_GT(c.get_r(), 0.0);
    EXPECT_GT(c.get_g(), 0.0);
    EXPECT_GT(c.get_b(), 0.0);
    EXPECT_EQ(c.to_discrete_r(2.2), 0);
    EXPECT_EQ(c.to_discrete_g(2.2), 0);
    EXPECT_EQ(c.to_discrete_b(2.2), 0);
  }

  // Verifica que la corrección gamma no altera el negro ni el blanco.
  TEST(ColorTest, GammaCorrectionPreservesBlackAndWhite) {
    color const black(0.0, 0.0, 0.0);
    color const white(1.0, 1.0, 1.0);
    double const gamma = 2.2;
    EXPECT_EQ(black.to_discrete_r(gamma), 0);
    EXPECT_EQ(black.to_discrete_g(gamma), 0);
    EXPECT_EQ(black.to_discrete_b(gamma), 0);
    EXPECT_EQ(white.to_discrete_r(gamma), 255);
    EXPECT_EQ(white.to_discrete_g(gamma), 255);
    EXPECT_EQ(white.to_discrete_b(gamma), 255);
  }

}  // namespace render
