#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <random>

namespace render {

  // Pruebas para matte_material

  // Comprueba que el constructor de matte_material inicializa la reflectancia.
  TEST(MatteMaterialTest, ConstructorInitializesReflectance) {
    vector const reflectance{0.5, 0.3, 0.8};
    matte_material const mat{reflectance};

    vector const result = mat.get_reflectance();
    EXPECT_DOUBLE_EQ(result.x, 0.5);
    EXPECT_DOUBLE_EQ(result.y, 0.3);
    EXPECT_DOUBLE_EQ(result.z, 0.8);
  }

  // Comprueba que el método get_type devuelve "matte".
  TEST(MatteMaterialTest, GetTypeReturnsMatte) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    EXPECT_EQ(mat.get_type(), "matte");
  }

  // Comprueba que get_reflectance() funciona correctamente para un material mate negro.
  TEST(MatteMaterialTest, ReflectanceBlack) {
    matte_material const mat{
      vector{0.0, 0.0, 0.0}
    };
    vector const result = mat.get_reflectance();
    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 0.0);
  }

  // Comprueba que get_reflectance() funciona correctamente para un material mate blanco.
  TEST(MatteMaterialTest, ReflectanceWhite) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    vector const result = mat.get_reflectance();
    EXPECT_DOUBLE_EQ(result.x, 1.0);
    EXPECT_DOUBLE_EQ(result.y, 1.0);
    EXPECT_DOUBLE_EQ(result.z, 1.0);
  }

  // Pruebas para metal_material

  // Comprueba que el constructor de metal_material inicializa la reflectancia y la difusión.
  TEST(MetalMaterialTest, ConstructorInitializesReflectanceAndDiffusion) {
    vector const reflectance{0.7, 0.6, 0.5};
    double const diffusion = 0.1;
    metal_material const metal{reflectance, diffusion};
    vector const result_refl = metal.get_reflectance();
    EXPECT_DOUBLE_EQ(result_refl.x, 0.7);
    EXPECT_DOUBLE_EQ(result_refl.y, 0.6);
    EXPECT_DOUBLE_EQ(result_refl.z, 0.5);
    EXPECT_DOUBLE_EQ(metal.get_diffusion(), 0.1);
  }

  // Comprueba que el método get_type devuelve "metal".
  TEST(MetalMaterialTest, GetTypeReturnsMetal) {
    metal_material const metal{
      vector{0.9, 0.9, 0.9},
      0.05
    };
    EXPECT_EQ(metal.get_type(), "metal");
  }

  // Comprueba que get_diffusion() funciona para un valor de difusión de 0.0
  TEST(MetalMaterialTest, DiffusionZero) {
    metal_material const metal{
      vector{0.8, 0.8, 0.8},
      0.0
    };
    EXPECT_DOUBLE_EQ(metal.get_diffusion(), 0.0);
  }

  // Comprueba que get_diffusion() funciona para un valor de difusión positivo
  TEST(MetalMaterialTest, DiffusionPositive) {
    metal_material const metal{
      vector{0.8, 0.8, 0.8},
      0.5
    };
    EXPECT_DOUBLE_EQ(metal.get_diffusion(), 0.5);
  }

  // Verifica que get_diffusion() funciona para un valor de difusión de 1.0
  TEST(MetalMaterialTest, HighDiffusion) {
    metal_material const metal{
      vector{1.0, 0.0, 0.0},
      1.0
    };
    EXPECT_DOUBLE_EQ(metal.get_diffusion(), 1.0);
  }

  // Pruebas para refractive_material

  // Comprueba que el constructor de refractive_material inicializa el índice de refracción.
  TEST(RefractiveMaterialTest, ConstructorInitializesRefractionIndex) {
    double const refraction_index = 1.5;
    refractive_material const refr{refraction_index};

    EXPECT_DOUBLE_EQ(refr.get_refraction_index(), 1.5);
  }

  // Comprueba que el método get_type devuelve "refractive".
  TEST(RefractiveMaterialTest, GetTypeReturnsRefractive) {
    refractive_material const refr{1.33};
    EXPECT_EQ(refr.get_type(), "refractive");
  }

  // Comprueba que get_reflectance() siempre devuelve (1,1,1) para un material refractivo.
  TEST(RefractiveMaterialTest, ReflectanceIsAlwaysOne) {
    refractive_material const refr{1.5};
    vector const result = refr.get_reflectance();
    EXPECT_DOUBLE_EQ(result.x, 1.0);
    EXPECT_DOUBLE_EQ(result.y, 1.0);
    EXPECT_DOUBLE_EQ(result.z, 1.0);
  }

  // Verifica que el índice de refracción se establece correctamente para el vidrio.
  TEST(RefractiveMaterialTest, RefractionIndexGlass) {
    refractive_material const refr{1.5};
    EXPECT_DOUBLE_EQ(refr.get_refraction_index(), 1.5);
  }

  // Verifica otro valor de índice de refracción, esta vez para agua.
  TEST(RefractiveMaterialTest, RefractionIndexWater) {
    refractive_material const refr{1.33};  // Agua
    EXPECT_DOUBLE_EQ(refr.get_refraction_index(), 1.33);
  }

  // Verifica otro valor de índice de refracción, esta vez para diamante.
  TEST(RefractiveMaterialTest, RefractionIndexDiamond) {
    refractive_material const refr{2.42};
    EXPECT_DOUBLE_EQ(refr.get_refraction_index(), 2.42);
  }

  // Pruebas de polimorfismo

  // Verifica que un puntero base a material puede apuntar a un objeto matte_material
  TEST(MaterialPolymorphismTest, MatteViaMaterialPointer) {
    std::unique_ptr<material> mat = std::make_unique<matte_material>(vector{0.5, 0.5, 0.5});
    EXPECT_EQ(mat->get_type(), "matte");
    vector const refl = mat->get_reflectance();
    EXPECT_DOUBLE_EQ(refl.x, 0.5);
    EXPECT_DOUBLE_EQ(refl.y, 0.5);
    EXPECT_DOUBLE_EQ(refl.z, 0.5);
  }

  // Verifica que un puntero base a material puede apuntar a un objeto metal_material
  TEST(MaterialPolymorphismTest, MetalViaMaterialPointer) {
    std::unique_ptr<material> mat = std::make_unique<metal_material>(vector{0.9, 0.9, 0.9}, 0.1);
    EXPECT_EQ(mat->get_type(), "metal");
  }

  TEST(MaterialPolymorphismTest, RefractiveViaMaterialPointer) {
    std::unique_ptr<material> mat = std::make_unique<refractive_material>(1.5);
    EXPECT_EQ(mat->get_type(), "refractive");
    vector const refl = mat->get_reflectance();
    EXPECT_DOUBLE_EQ(refl.x, 1.0);
    EXPECT_DOUBLE_EQ(refl.y, 1.0);
    EXPECT_DOUBLE_EQ(refl.z, 1.0);
  }

}  // namespace render

// Para tests de scatter
class ScatterTest : public ::testing::Test {
protected:
  ScatterTest() : rng(12'345) { }

  // Rayo de entrada (mirando hacia -Z)
  render::ray r_in{
    render::vector{0, 0,  5},
    render::vector{0, 0, -1}
  };

  // Registro de hit (superficie plana en (0,0,0) con normal hacia +Z)
  render::hit_record rec{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1},
    nullptr, 5.0, true
  };
  render::ray scattered;
  std::mt19937_64 rng;
};

// Comprueba la función 'scatter' para un material mate
TEST_F(ScatterTest, MatteScatter) {
  render::matte_material const mat{
    render::vector{0.8, 0.5, 0.3}
  };
  rec.mat_ptr                         = &mat;
  render::scatter_result const result = mat.scatter(r_in, rec, scattered, rng);
  EXPECT_TRUE(result.scattered);
  // La atenuación debe ser el color del material
  EXPECT_DOUBLE_EQ(result.attenuation.x, 0.8);
  EXPECT_DOUBLE_EQ(result.attenuation.y, 0.5);
  EXPECT_DOUBLE_EQ(result.attenuation.z, 0.3);

  // El rayo disperso debe originarse en el punto de impacto
  EXPECT_DOUBLE_EQ(scattered.get_origin().x, rec.point.x);
  EXPECT_DOUBLE_EQ(scattered.get_origin().y, rec.point.y);
  EXPECT_DOUBLE_EQ(scattered.get_origin().z, rec.point.z);
}

// Prueba 'scatter' para un material metálico sin difusión
TEST_F(ScatterTest, MetalScatterNoDiffusion) {
  render::metal_material const mat{
    render::vector{1.0, 0.9, 0.8},
    0.0
  };
  rec.mat_ptr                         = &mat;
  render::scatter_result const result = mat.scatter(r_in, rec, scattered, rng);
  EXPECT_TRUE(result.scattered);
  // La atenuación debe ser el color del material
  EXPECT_DOUBLE_EQ(result.attenuation.x, 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.y, 0.9);
  EXPECT_DOUBLE_EQ(result.attenuation.z, 0.8);
  // El rayo disperso debe ser el reflejo perfecto
  render::vector const dir = scattered.get_direction().normalized();
  EXPECT_NEAR(dir.x, 0.0, 1e-9);
  EXPECT_NEAR(dir.y, 0.0, 1e-9);
  EXPECT_NEAR(dir.z, 1.0, 1e-9);
}

// Prueba 'scatter' para un material refractivo
TEST_F(ScatterTest, RefractiveScatter) {
  render::refractive_material const mat{1.5};
  rec.mat_ptr                         = &mat;
  render::scatter_result const result = mat.scatter(r_in, rec, scattered, rng);
  EXPECT_TRUE(result.scattered);
  // La atenuación de refracción es siempre (1, 1, 1)
  EXPECT_DOUBLE_EQ(result.attenuation.x, 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.y, 1.0);
  EXPECT_DOUBLE_EQ(result.attenuation.z, 1.0);
}
