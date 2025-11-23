#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <numbers>

namespace render {

  // Pruebas para sphere

  // Comprueba que el constructor de sphere inicializa correctamente el centro, el radio y el
  // material.
  TEST(SphereTest, ConstructorInitializesProperties) {
    matte_material const mat{
      vector{0.5, 0.5, 0.5}
    };
    vector const center{1.0, 2.0, 3.0};
    double const radius = 5.0;
    sphere const sph{center, radius, &mat};
    vector const result_center = sph.get_center();
    EXPECT_DOUBLE_EQ(result_center.x, 1.0);
    EXPECT_DOUBLE_EQ(result_center.y, 2.0);
    EXPECT_DOUBLE_EQ(result_center.z, 3.0);
    EXPECT_DOUBLE_EQ(sph.get_radius(), 5.0);
  }

  // Verifica que el método get_type devuelve "sphere".
  TEST(SphereTest, GetTypeReturnsSphere) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };
    EXPECT_EQ(sph.get_type(), "sphere");
  }

  // Verifica que el método get_material devuelve el material correcto.
  TEST(SphereTest, GetMaterialReturnsCorrectMaterial) {
    matte_material const mat{
      vector{0.8, 0.2, 0.4}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    material const * result_mat = sph.get_material();
    EXPECT_EQ(result_mat->get_type(), "matte");

    vector const refl = result_mat->get_reflectance();
    EXPECT_DOUBLE_EQ(refl.x, 0.8);
    EXPECT_DOUBLE_EQ(refl.y, 0.2);
    EXPECT_DOUBLE_EQ(refl.z, 0.4);
  }

  // Prueba la construcción de una esfera con un radio muy pequeño.
  TEST(SphereTest, SmallRadius) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      0.01, &mat
    };
    EXPECT_DOUBLE_EQ(sph.get_radius(), 0.01);
  }

  // Lo mismo pero con un radio muy grande.
  TEST(SphereTest, LargeRadius) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1000.0, &mat
    };
    EXPECT_DOUBLE_EQ(sph.get_radius(), 1000.0);
  }

  // Prueba la construcción de una esfera con coordenadas negativas.
  TEST(SphereTest, NegativeCoordinates) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{-5.0, -10.0, -15.0},
      2.5, &mat
    };
    vector const center = sph.get_center();
    EXPECT_DOUBLE_EQ(center.x, -5.0);
    EXPECT_DOUBLE_EQ(center.y, -10.0);
    EXPECT_DOUBLE_EQ(center.z, -15.0);
  }

  // Pruebas con diferentes tipos de material asociados a la esfera.
  TEST(SphereTest, WithMetalMaterial) {
    metal_material const mat{
      vector{0.9, 0.9, 0.9},
      0.1
    };
    sphere const sph{
      vector{1.0, 1.0, 1.0},
      3.0, &mat
    };

    material const * result_mat = sph.get_material();
    EXPECT_EQ(result_mat->get_type(), "metal");
  }

  // Pruebas con material refractivo asociado a la esfera.
  TEST(SphereTest, WithRefractiveMaterial) {
    refractive_material const mat{1.5};
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      2.0, &mat
    };

    material const * result_mat = sph.get_material();
    EXPECT_EQ(result_mat->get_type(), "refractive");
  }

  // Pruebas para cylinder

  // Comprueba que el constructor del cilindro inicializa correctamente el centro, el radio, el eje
  // y el material.
  TEST(CylinderTest, ConstructorInitializesProperties) {
    matte_material const mat{
      vector{0.5, 0.5, 0.5}
    };
    vector const center{1.0, 2.0, 3.0};
    double const radius = 2.0;
    vector const axis{0.0, 0.0, 4.0};
    cylinder const cyl{center, radius, axis, &mat};
    vector const result_center = cyl.get_center();
    EXPECT_DOUBLE_EQ(result_center.x, 1.0);
    EXPECT_DOUBLE_EQ(result_center.y, 2.0);
    EXPECT_DOUBLE_EQ(result_center.z, 3.0);
    EXPECT_DOUBLE_EQ(cyl.get_radius(), 2.0);
  }

  // Verifica que el método get_type devuelve "cylinder".
  TEST(CylinderTest, GetTypeReturnsCylinder) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 1.0, 0.0},
      &mat
    };

    EXPECT_EQ(cyl.get_type(), "cylinder");
  }

  // Verifica que el eje se almacena correctamente.
  TEST(CylinderTest, AxisStoredCorrectly) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    vector const axis{1.0, 2.0, 3.0};
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, axis, &mat
    };

    vector const result_axis = cyl.get_axis();
    EXPECT_DOUBLE_EQ(result_axis.x, 1.0);
    EXPECT_DOUBLE_EQ(result_axis.y, 2.0);
    EXPECT_DOUBLE_EQ(result_axis.z, 3.0);
  }

  // Verifica que la altura se calcula correctamente a partir de la magnitud del eje.
  TEST(CylinderTest, HeightCalculatedFromAxisMagnitude) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    vector const axis{3.0, 4.0, 0.0};  // Magnitud = 5.0
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, axis, &mat
    };

    EXPECT_DOUBLE_EQ(cyl.get_height(), 5.0);
  }

  // Comprueba que la altura  se calcula correctamente a partir de la magnitud del eje
  TEST(CylinderTest, HeightCalculatedCorrectly) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    vector const axis{0.0, 0.0, 10.0};
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      2.0, axis, &mat
    };

    EXPECT_DOUBLE_EQ(cyl.get_height(), 10.0);
  }

  // Verifica la altura cuando el eje es diagonal.
  TEST(CylinderTest, HeightWithDiagonalAxis) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    vector const axis{1.0, 1.0, 1.0};  // Magnitud = sqrt(3)
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, axis, &mat
    };
    double const expected_height = std::numbers::sqrt3;
    EXPECT_NEAR(cyl.get_height(), expected_height, 1e-10);
  }

  // Verifica que el método get_material devuelve el material correcto.
  TEST(CylinderTest, GetMaterialReturnsCorrectMaterial) {
    metal_material const mat{
      vector{0.7, 0.7, 0.7},
      0.05
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 1.0, 0.0},
      &mat
    };

    material const * result_mat = cyl.get_material();
    EXPECT_EQ(result_mat->get_type(), "metal");
  }

  // Prueba la construcción de un cilindro con un radio pequeño.
  TEST(CylinderTest, SmallRadiusCylinder) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      0.1, vector{0.0, 5.0, 0.0},
      &mat
    };

    EXPECT_DOUBLE_EQ(cyl.get_radius(), 0.1);
  }

  // Lo mismo pero con radio grande.
  TEST(CylinderTest, WithRefractiveMaterial) {
    refractive_material const mat{1.33};
    cylinder const cyl{
      vector{1.0, 1.0, 1.0},
      2.0, vector{0.0, 0.0, 8.0},
      &mat
    };

    material const * result_mat = cyl.get_material();
    EXPECT_EQ(result_mat->get_type(), "refractive");
  }

  // Pruebas de polimorfismo

  // Verifica que un puntero base a object puede apuntar a un objeto sphere
  TEST(ObjectPolymorphismTest, SphereViaObjectPointer) {
    matte_material const mat{
      vector{0.5, 0.5, 0.5}
    };
    std::unique_ptr<object> obj = std::make_unique<sphere>(vector{1.0, 2.0, 3.0}, 5.0, &mat);
    EXPECT_EQ(obj->get_type(), "sphere");
    EXPECT_DOUBLE_EQ(obj->get_radius(), 5.0);
    vector const center = obj->get_center();
    EXPECT_DOUBLE_EQ(center.x, 1.0);
    EXPECT_DOUBLE_EQ(center.y, 2.0);
    EXPECT_DOUBLE_EQ(center.z, 3.0);
  }

  // Verifica que un puntero base a object puede apuntar a un objeto cylinder
  TEST(ObjectPolymorphismTest, CylinderViaObjectPointer) {
    metal_material const mat{
      vector{0.9, 0.9, 0.9},
      0.1
    };
    std::unique_ptr<object> obj =
        std::make_unique<cylinder>(vector{0.0, 0.0, 0.0}, 2.0, vector{0.0, 0.0, 10.0}, &mat);

    EXPECT_EQ(obj->get_type(), "cylinder");
    EXPECT_DOUBLE_EQ(obj->get_radius(), 2.0);
  }

  // Verifica el acceso al material a través de un puntero base a object
  TEST(ObjectPolymorphismTest, MaterialAccessThroughObject) {
    refractive_material const mat{1.5};
    std::unique_ptr<object> obj = std::make_unique<sphere>(vector{0.0, 0.0, 0.0}, 1.0, &mat);

    material const * result_mat = obj->get_material();
    EXPECT_EQ(result_mat->get_type(), "refractive");

    vector const refl = result_mat->get_reflectance();
    EXPECT_DOUBLE_EQ(refl.x, 1.0);
    EXPECT_DOUBLE_EQ(refl.y, 1.0);
    EXPECT_DOUBLE_EQ(refl.z, 1.0);
  }

  // Verifica el polimorfismo con una mezcla de tipos de objetos
  TEST(ObjectPolymorphismTest, MixedObjectTypes) {
    matte_material const mat1{
      vector{0.8, 0.2, 0.2}
    };
    metal_material const mat2{
      vector{0.9, 0.9, 0.9},
      0.05
    };

    std::unique_ptr<object> obj1 = std::make_unique<sphere>(vector{0.0, 0.0, 0.0}, 1.0, &mat1);
    std::unique_ptr<object> obj2 =
        std::make_unique<cylinder>(vector{5.0, 5.0, 5.0}, 2.0, vector{0.0, 10.0, 0.0}, &mat2);

    EXPECT_EQ(obj1->get_type(), "sphere");
    EXPECT_EQ(obj2->get_type(), "cylinder");
    EXPECT_EQ(obj1->get_material()->get_type(), "matte");
    EXPECT_EQ(obj2->get_material()->get_type(), "metal");
  }

  // Comprueba una esfera construida en el origen
  TEST(ObjectEdgeCaseTest, SphereAtOrigin) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    vector const center = sph.get_center();
    EXPECT_DOUBLE_EQ(center.x, 0.0);
    EXPECT_DOUBLE_EQ(center.y, 0.0);
    EXPECT_DOUBLE_EQ(center.z, 0.0);
  }

  // Comprueba un cilindro con eje unitario
  TEST(ObjectEdgeCaseTest, CylinderWithUnitAxis) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 1.0, 0.0},
      &mat
    };

    EXPECT_DOUBLE_EQ(cyl.get_height(), 1.0);
  }

  // Comprueba una esfera con un radio muy grande
  TEST(ObjectEdgeCaseTest, VeryLargeSphere) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      10000.0, &mat
    };

    EXPECT_DOUBLE_EQ(sph.get_radius(), 10000.0);
  }

  // Pruebas de intersección (hit) para sphere

  // Verifica que si un rayo que apunta directamente al centro de la esfera la intersecta
  // correctamente.
  TEST(SphereHitTest, RayHitsSphereFromOutside) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    ray const r{
      vector{0.0, 0.0, -5.0},
      vector{0.0, 0.0,  1.0}
    };
    hit_record rec;

    EXPECT_TRUE(sph.hit(r, 0.0, 100.0, rec));
    EXPECT_NEAR(rec.t, 4.0, 1e-10);
    EXPECT_TRUE(rec.front_face);
  }

  // Verifica que un rayo que pasa cerca de la esfera pero no la intersecta devuelve false.
  TEST(SphereHitTest, RayMissesSphere) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    ray const r{
      vector{5.0, 0.0, 0.0},
      vector{0.0, 0.0, 1.0}
    };
    hit_record rec;

    EXPECT_FALSE(sph.hit(r, 0.0, 100.0, rec));
  }

  // Verifica que un rayo que se origina dentro de la esfera la intersecta correctamente.
  TEST(SphereHitTest, RayOriginatesInsideSphere) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      2.0, &mat
    };

    ray const r{
      vector{0.0, 0.0, 0.0},
      vector{1.0, 0.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(sph.hit(r, 0.0, 100.0, rec));
    EXPECT_NEAR(rec.t, 2.0, 1e-10);
    EXPECT_FALSE(rec.front_face);
  }

  // Comprueba que la intersección se descarta si está fuera del rango [t_min, t_max].
  TEST(SphereHitTest, IntersectionOutsideTRange) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    ray const r{
      vector{0.0, 0.0, -5.0},
      vector{0.0, 0.0,  1.0}
    };
    hit_record rec;

    EXPECT_FALSE(sph.hit(r, 10.0, 100.0, rec));
  }

  // Verifica que la normal en el punto de intersección apunta hacia afuera de la esfera.
  TEST(SphereHitTest, NormalPointsOutward) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    ray const r{
      vector{ 2.0, 0.0, 0.0},
      vector{-1.0, 0.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(sph.hit(r, 0.0, 100.0, rec));

    vector const expected_normal{1.0, 0.0, 0.0};
    EXPECT_NEAR(rec.normal.x, expected_normal.x, 1e-10);
    EXPECT_NEAR(rec.normal.y, expected_normal.y, 1e-10);
    EXPECT_NEAR(rec.normal.z, expected_normal.z, 1e-10);
  }

  // Verifica que el material registrado en hit_record es el correcto.
  TEST(SphereHitTest, MaterialIsCorrect) {
    metal_material const mat{
      vector{0.9, 0.9, 0.9},
      0.1
    };
    sphere const sph{
      vector{0.0, 0.0, 0.0},
      1.0, &mat
    };

    ray const r{
      vector{0.0, 0.0, -5.0},
      vector{0.0, 0.0,  1.0}
    };
    hit_record rec;

    EXPECT_TRUE(sph.hit(r, 0.0, 100.0, rec));
    EXPECT_EQ(rec.mat_ptr->get_type(), "metal");
  }

  // Pruebas de intersección (hit) para cylinder

  // Verifica que un rayo intersecta correctamente la superficie curva del cilindro.
  TEST(CylinderHitTest, RayHitsCurvedSurface) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    ray const r{
      vector{ 5.0, 0.0, 0.0},
      vector{-1.0, 0.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(cyl.hit(r, 0.0, 100.0, rec));
    EXPECT_NEAR(rec.t, 4.0, 1e-6);
  }

  // Verifica que un rayo que no intersecta el cilindro devuelve false.
  TEST(CylinderHitTest, RayMissesCylinder) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    ray const r{
      vector{5.0, 0.0, 0.0},
      vector{0.0, 1.0, 0.0}
    };
    hit_record rec;

    EXPECT_FALSE(cyl.hit(r, 0.0, 100.0, rec));
  }

  // Verifica que un rayo que intersecta la base superior del cilindro lo detecta correctamente.
  TEST(CylinderHitTest, RayHitsTopCap) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    ray const r{
      vector{0.0,  5.0, 0.0},
      vector{0.0, -1.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(cyl.hit(r, 0.0, 100.0, rec));
    EXPECT_NEAR(rec.point.y, 2.0, 1e-6);
  }

  // Verifica que un rayo que intersecta la base inferior del cilindro lo detecta correctamente.
  TEST(CylinderHitTest, RayHitsBottomCap) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    ray const r{
      vector{0.0, -5.0, 0.0},
      vector{0.0,  1.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(cyl.hit(r, 0.0, 100.0, rec));
    EXPECT_NEAR(rec.point.y, -2.0, 1e-6);
  }

  // Verifica que un rayo paralelo al eje del cilindro lo intersecta correctamente.
  TEST(CylinderHitTest, RayParallelToAxis) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    ray const r{
      vector{0.5, -5.0, 0.0},
      vector{0.0,  1.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(cyl.hit(r, 0.0, 100.0, rec));
  }

  // Verifica que el material registrado en hit_record es el correcto.
  TEST(CylinderHitTest, MaterialIsCorrect) {
    refractive_material const mat{1.5};
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    ray const r{
      vector{ 5.0, 0.0, 0.0},
      vector{-1.0, 0.0, 0.0}
    };
    hit_record rec;

    EXPECT_TRUE(cyl.hit(r, 0.0, 100.0, rec));
    EXPECT_EQ(rec.mat_ptr->get_type(), "refractive");
  }

  // Verifica que se devuelve la intersección más cercana cuando hay múltiples.
  TEST(CylinderHitTest, ClosestIntersection) {
    matte_material const mat{
      vector{1.0, 1.0, 1.0}
    };
    cylinder const cyl{
      vector{0.0, 0.0, 0.0},
      1.0, vector{0.0, 4.0, 0.0},
      &mat
    };

    // Rayo que atraviesa el cilindro
    ray const r{
      vector{0.0, 0.0, -5.0},
      vector{0.0, 0.0,  1.0}
    };
    hit_record rec;

    EXPECT_TRUE(cyl.hit(r, 0.0, 100.0, rec));
    // Debe devolver la intersección más cercana
    EXPECT_NEAR(rec.t, 4.0, 1e-6);
  }

}  // namespace render
