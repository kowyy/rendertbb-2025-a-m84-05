#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "vector.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>

class MockObject : public render::object {
public:
  MockObject(bool will_hit, double hit_t, render::material const * mat)
      : render::object(mat), m_will_hit(will_hit), m_hit_t(hit_t) { }

  [[nodiscard]] bool hit(render::ray const &, double t_min, double t_max,
                         render::hit_record & rec) const override {
    //  Verificar rango [t_min, t_max]
    if (m_will_hit and m_hit_t >= t_min and m_hit_t <= t_max) {
      rec.t          = m_hit_t;
      rec.mat_ptr    = get_material();
      rec.point      = render::vector{0, 0, m_hit_t};  // Punto de intersección
      rec.normal     = render::vector{0, 0, 1};        // Normal ficticia
      rec.front_face = true;
      return true;
    }
    return false;
  }

  [[nodiscard]] std::string get_type() const override { return "mock"; }

  [[nodiscard]] render::vector get_center() const override { return render::vector{0, 0, 0}; }

  [[nodiscard]] double get_radius() const override { return 0.0; }

private:
  bool m_will_hit;
  double m_hit_t;
};

// Comprueba que una escena creada por defecto no produce ninguna intersección.
TEST(SceneTest, DefaultConstructor) {
  render::scene const scn;
  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  EXPECT_FALSE(scn.hit(r, 0.001, 100.0, rec));
}

// Verifica que se puede añadir un material a la escena y recuperarlo por su nombre.
TEST(SceneTest, AddAndRetrieveMaterial) {
  render::scene scn;
  auto mat1 = std::make_unique<render::matte_material>(render::vector{0.8, 0.8, 0.8});
  render::material const * mat1_ptr = mat1.get();

  scn.add_material("mat1", std::move(mat1));

  render::material const * retrieved = scn.get_material("mat1");
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved, mat1_ptr);
  EXPECT_EQ(retrieved->get_type(), "matte");
}

// Comprueba que 'get_material' devuelve 'nullptr' si se solicita un nombre de material que no
// existe.
TEST(SceneTest, GetNonExistentMaterial) {
  render::scene const scn;
  render::material const * mat = scn.get_material("nonexistent");
  EXPECT_EQ(mat, nullptr);
}

// Verifica que se pueden añadir múltiples materiales y recuperarlos correctamente.
TEST(SceneTest, AddMultipleMaterials) {
  render::scene scn;

  auto mat1 = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  auto mat2 = std::make_unique<render::metal_material>(render::vector{0, 1, 0}, 0.1);
  auto mat3 = std::make_unique<render::refractive_material>(1.5);

  scn.add_material("red_matte", std::move(mat1));
  scn.add_material("green_metal", std::move(mat2));
  scn.add_material("glass", std::move(mat3));

  EXPECT_NE(scn.get_material("red_matte"), nullptr);
  EXPECT_NE(scn.get_material("green_metal"), nullptr);
  EXPECT_NE(scn.get_material("glass"), nullptr);
  EXPECT_EQ(scn.get_material("red_matte")->get_type(), "matte");
  EXPECT_EQ(scn.get_material("green_metal")->get_type(), "metal");
  EXPECT_EQ(scn.get_material("glass")->get_type(), "refractive");
}

// Tests de Hit Logic

// Verifica que se produce una intersección cuando hay un solo objeto que intersecta el rayo.
TEST(SceneTest, HitWithSingleObject) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 5.0, mat_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  ASSERT_TRUE(scn.hit(r, 0.001, 100.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 5.0);
  EXPECT_EQ(rec.mat_ptr, mat_ptr);
}

// Verifica que se devuelve la intersección más cercana cuando hay múltiples objetos.
TEST(SceneTest, HitReturnsClosestObject) {
  render::scene scn;
  auto mat1 = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  auto mat2 = std::make_unique<render::matte_material>(render::vector{0, 1, 0});
  auto mat3 = std::make_unique<render::matte_material>(render::vector{0, 0, 1});

  render::material const * mat1_ptr = mat1.get();
  render::material const * mat2_ptr = mat2.get();
  render::material const * mat3_ptr = mat3.get();

  scn.add_material("mat1", std::move(mat1));
  scn.add_material("mat2", std::move(mat2));
  scn.add_material("mat3", std::move(mat3));

  scn.add_object(std::make_unique<MockObject>(true, 10.0, mat1_ptr));
  scn.add_object(std::make_unique<MockObject>(true, 2.0, mat2_ptr));  // Más cercano
  scn.add_object(std::make_unique<MockObject>(true, 15.0, mat3_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  ASSERT_TRUE(scn.hit(r, 0.001, 100.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 2.0);  // Debe devolver el más cercano
  EXPECT_EQ(rec.mat_ptr, mat2_ptr);
}

// Verifica que los objetos que no intersectan el rayo son ignorados.
TEST(SceneTest, HitIgnoresObjectsThatMiss) {
  render::scene scn;
  auto mat1 = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  auto mat2 = std::make_unique<render::matte_material>(render::vector{0, 1, 0});

  render::material const * mat1_ptr = mat1.get();
  render::material const * mat2_ptr = mat2.get();

  scn.add_material("mat1", std::move(mat1));
  scn.add_material("mat2", std::move(mat2));

  scn.add_object(std::make_unique<MockObject>(false, 10.0, mat1_ptr));  // No hit
  scn.add_object(std::make_unique<MockObject>(true, 5.0, mat2_ptr));    // Hit
  scn.add_object(std::make_unique<MockObject>(false, 3.0, mat1_ptr));   // No hit

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  ASSERT_TRUE(scn.hit(r, 0.001, 100.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 5.0);
  EXPECT_EQ(rec.mat_ptr, mat2_ptr);
}

// Verifica que no se produce ninguna intersección si todos los objetos fallan.
TEST(SceneTest, NoHitWhenAllObjectsMiss) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(false, 5.0, mat_ptr));
  scn.add_object(std::make_unique<MockObject>(false, 10.0, mat_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  EXPECT_FALSE(scn.hit(r, 0.001, 100.0, rec));
}

// Verifica que una escena vacía no produce ninguna intersección.
TEST(SceneTest, EmptySceneReturnsNoHit) {
  render::scene const scn;
  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  EXPECT_FALSE(scn.hit(r, 0.001, 100.0, rec));
}

// Tests de Límites t_min y t_max

// Verifica que las intersecciones antes de t_min son ignoradas.
TEST(SceneTest, IgnoresHitsBeforeTmin) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 0.01, mat_ptr));  // t < t_min

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  // t=0.01 está antes de t_min=0.1, debe ignorarse
  EXPECT_FALSE(scn.hit(r, 0.1, 100.0, rec));
}

// Verifica que las intersecciones después de t_max son ignoradas.
TEST(SceneTest, IgnoresHitsAfterTmax) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 50.0, mat_ptr));  // t > t_max

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  // t=50.0 está después de t_max=40.0, debe ignorarse
  EXPECT_FALSE(scn.hit(r, 0.001, 40.0, rec));
}

// Verifica que una intersección exactamente en t_min es válida.
TEST(SceneTest, HitExactlyAtTmin) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 0.1, mat_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  // t=0.1 == t_min, debe encontrarse
  ASSERT_TRUE(scn.hit(r, 0.1, 100.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 0.1);
}

// Verifica que una intersección exactamente en t_max es válida.
TEST(SceneTest, HitExactlyAtTmax) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 40.0, mat_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  // t=40.0 == t_max, debe encontrarse
  ASSERT_TRUE(scn.hit(r, 0.001, 40.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 40.0);
}

// Comprueba que se devuelve la intersección más cercana dentro de los límites t_min y t_max.
TEST(SceneTest, ReturnsClosestHitWithinRange) {
  render::scene scn;
  auto mat1 = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  auto mat2 = std::make_unique<render::matte_material>(render::vector{0, 1, 0});
  auto mat3 = std::make_unique<render::matte_material>(render::vector{0, 0, 1});

  render::material const * mat1_ptr = mat1.get();
  render::material const * mat2_ptr = mat2.get();
  render::material const * mat3_ptr = mat3.get();

  scn.add_material("mat1", std::move(mat1));
  scn.add_material("mat2", std::move(mat2));
  scn.add_material("mat3", std::move(mat3));

  scn.add_object(std::make_unique<MockObject>(true, 0.5, mat1_ptr));    // Antes de t_min
  scn.add_object(std::make_unique<MockObject>(true, 5.0, mat2_ptr));    // Dentro de rango
  scn.add_object(std::make_unique<MockObject>(true, 150.0, mat3_ptr));  // Después de t_max

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  // Solo el objeto en t=5.0 está en rango [1.0, 100.0]
  ASSERT_TRUE(scn.hit(r, 1.0, 100.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 5.0);
  EXPECT_EQ(rec.mat_ptr, mat2_ptr);
}

// Tests de Gestión de Materiales

// Verifica que añadir un material con un nombre ya existente sobrescribe el anterior.
TEST(SceneTest, OverwriteMaterialWithSameName) {
  render::scene scn;

  auto mat1 = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  auto mat2 = std::make_unique<render::metal_material>(render::vector{0, 1, 0}, 0.5);

  scn.add_material("shared_name", std::move(mat1));
  scn.add_material("shared_name", std::move(mat2));

  render::material const * retrieved = scn.get_material("shared_name");
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->get_type(), "metal");  // Debe ser el último añadido
}

// Verifica que los materiales siguen accesibles después de añadir varios objetos.
TEST(SceneTest, MaterialsSurviveObjectAddition) {
  render::scene scn;

  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));

  for (int i = 0; i < 100; ++i) {
    scn.add_object(std::make_unique<MockObject>(false, static_cast<double>(i), mat_ptr));
  }

  // El material debe seguir accesible
  EXPECT_EQ(scn.get_material("mat"), mat_ptr);
}

// Prueba la lógica de 'hit' con un rango [t_min, t_max] muy pequeño.
TEST(SceneTest, VerySmallTminTmax) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 0.0001, mat_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  ASSERT_TRUE(scn.hit(r, 0.00001, 0.001, rec));
  EXPECT_DOUBLE_EQ(rec.t, 0.0001);
}

// Prueba la lógica de 'hit' con un rango [t_min, t_max] muy grande.
TEST(SceneTest, VeryLargeTmax) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));
  scn.add_object(std::make_unique<MockObject>(true, 1e6, mat_ptr));

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  ASSERT_TRUE(scn.hit(r, 0.001, 1e9, rec));
  EXPECT_DOUBLE_EQ(rec.t, 1e6);
}

// Comprueba que la escena maneja una gran cantidad de objetos y encuentra el correcto.
TEST(SceneTest, ManyObjectsPerformance) {
  render::scene scn;
  auto mat = std::make_unique<render::matte_material>(render::vector{1, 0, 0});
  render::material const * mat_ptr = mat.get();

  scn.add_material("mat", std::move(mat));

  // Añadir 1000 objetos
  for (int i = 0; i < 1'000; ++i) {
    scn.add_object(std::make_unique<MockObject>(i == 500,  // Solo el objeto 500 hace hit
                                                static_cast<double>(i), mat_ptr));
  }

  render::ray const r{
    render::vector{0, 0, 0},
    render::vector{0, 0, 1}
  };
  render::hit_record rec;

  ASSERT_TRUE(scn.hit(r, 0.001, 1000.0, rec));
  EXPECT_DOUBLE_EQ(rec.t, 500.0);
}
