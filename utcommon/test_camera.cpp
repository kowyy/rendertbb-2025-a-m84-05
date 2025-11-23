#include "camera.hpp"
#include "config.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <numbers>
#include <stdexcept>

namespace render {

  // Comprueba que el constructor de la cámara no lanza ninguna excepción cuando se utiliza una
  // configuración por defecto.
  TEST(CameraTest, ConstructorWithDefaultConfig) {
    config const cfg;
    EXPECT_NO_THROW(camera const cam(cfg));
  }

  // Comprueba que la cámara lanza una excepción si la posición es igual al objetivo.
  TEST(CameraTest, ThrowsWhenPositionEqualsTarget) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 0));
    cfg.set_camera_target(vector(0, 0, 0));
    EXPECT_THROW(camera const cam(cfg), std::runtime_error);
  }

  // Comprueba que la cámara lanza una excepción si la dirección de "norte" es paralela a la
  // dirección de visión.
  TEST(CameraTest, ThrowsWhenNorthParallelToViewDirection) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 0));
    cfg.set_camera_target(vector(0, 0, -1));
    cfg.set_camera_north(vector(0, 0, 1));
    EXPECT_THROW(camera const cam(cfg), std::runtime_error);
  }

  // Comprueba que la cámara lanza una excepción si la dirección de "norte" es antiparalela a la
  // dirección de visión.
  TEST(CameraTest, ThrowsWhenNorthAntiparallelToViewDirection) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 0));
    cfg.set_camera_target(vector(0, 0, -1));
    cfg.set_camera_north(vector(0, 0, -1));
    EXPECT_THROW(camera const cam(cfg), std::runtime_error);
  }

  // Comprueba que el vector focal se calcula correctamente.
  TEST(CameraTest, FocalVectorCalculation) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 10));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    camera const cam(cfg);
    ray const r_center  = cam.get_ray(0.5, 0.5);
    vector const origin = r_center.get_origin();
    EXPECT_DOUBLE_EQ(origin.x, 0.0);
    EXPECT_DOUBLE_EQ(origin.y, 0.0);
    EXPECT_DOUBLE_EQ(origin.z, 10.0);
  }

  // Comprueba la altura del viewport con un FOV de 90 grados.
  TEST(CameraTest, ViewportHeightWithFOV90) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 1));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    cfg.set_aspect_ratio(1, 1);
    camera const cam(cfg);
    ray const r_ll             = cam.get_ray(0.0, 0.0);
    ray const r_ur             = cam.get_ray(1.0, 1.0);
    vector const dir_ll        = r_ll.get_direction().normalized();
    vector const dir_ur        = r_ur.get_direction().normalized();
    constexpr double sqrt3_inv = std::numbers::inv_sqrt3;
    EXPECT_NEAR(dir_ll.x, -sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ll.y, sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ll.z, -sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ur.x, sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ur.y, -sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ur.z, -sqrt3_inv, 1e-3);
  }

  // Verifica que el ángulo entre el rayo central y el rayo del borde superior
  // sea la mitad del FOV (campo de visión)
  TEST(CameraTest, ViewportHeightWithFOV60) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 2));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(60.0);
    camera const cam(cfg);
    ray const r_center      = cam.get_ray(0.5, 0.5);
    ray const r_top         = cam.get_ray(0.5, 1.0);
    vector const dir_center = r_center.get_direction().normalized();
    vector const dir_top    = r_top.get_direction().normalized();
    double const cos_angle  = vector::dot(dir_center, dir_top);
    double const angle_rad  = std::acos(cos_angle);
    double const angle_deg  = angle_rad * 180.0 / std::numbers::pi;
    EXPECT_NEAR(angle_deg, 30.0, 2.0);
  }

  // Comprueba que los vectores de la base de la cámara son ortonormales, verificando que el
  // producto escalar entre ellos sea cero.
  TEST(CameraTest, VectorDirectorsOrthonormal) {
    config cfg;
    cfg.set_camera_position(vector(1, 2, 3));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(45.0);
    camera const cam(cfg);
    ray const r1                 = cam.get_ray(0.5, 0.5);
    ray const r2                 = cam.get_ray(0.6, 0.5);
    ray const r3                 = cam.get_ray(0.5, 0.6);
    vector const d1              = r1.get_direction();
    vector const d2              = r2.get_direction();
    vector const d3              = r3.get_direction();
    vector const horizontal_step = d2 - d1;
    vector const vertical_step   = d3 - d1;
    double const dot_product     = vector::dot(horizontal_step, vertical_step);
    EXPECT_NEAR(dot_product, 0.0, 1e-6);
  }

  // Prueba que el rayo central apunta correctamente hacia el objetivo de la cámara.
  TEST(CameraTest, CenterRayPointsToTarget) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 10));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    cfg.set_aspect_ratio(16, 9);
    camera const cam(cfg);
    ray const r = cam.get_ray(0.5, 0.5);
    EXPECT_DOUBLE_EQ(r.get_origin().x, 0.0);
    EXPECT_DOUBLE_EQ(r.get_origin().y, 0.0);
    EXPECT_DOUBLE_EQ(r.get_origin().z, 10.0);
    vector const direction = r.get_direction().normalized();
    EXPECT_NEAR(direction.x, 0.0, 1e-3);
    EXPECT_NEAR(direction.y, 0.0, 1e-3);
    EXPECT_NEAR(direction.z, -1.0, 1e-3);
  }

  // Comprueba los rayos en las esquinas del viewport con una relación de aspecto cuadrada.
  TEST(CameraTest, CornerRaysWithSquareAspect) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 1));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    cfg.set_aspect_ratio(1, 1);
    camera const cam(cfg);
    ray const r_ll             = cam.get_ray(0.0, 0.0);
    ray const r_ur             = cam.get_ray(1.0, 1.0);
    constexpr double sqrt3_inv = std::numbers::inv_sqrt3;
    vector const dir_ll        = r_ll.get_direction().normalized();
    EXPECT_NEAR(dir_ll.x, -sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ll.y, sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ll.z, -sqrt3_inv, 1e-3);
    vector const dir_ur = r_ur.get_direction().normalized();
    EXPECT_NEAR(dir_ur.x, sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ur.y, -sqrt3_inv, 1e-3);
    EXPECT_NEAR(dir_ur.z, -sqrt3_inv, 1e-3);
  }

  // Verifica que el origen de todos los rayos generados sea la posición de la cámara.
  TEST(CameraTest, RayOriginIsAlwaysCameraPosition) {
    config cfg;
    cfg.set_camera_position(vector(5, 10, 15));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    camera const cam(cfg);
    ray const r1 = cam.get_ray(0.0, 0.0);
    ray const r2 = cam.get_ray(0.5, 0.5);
    ray const r3 = cam.get_ray(1.0, 1.0);
    for (auto const & r : {r1, r2, r3}) {
      EXPECT_DOUBLE_EQ(r.get_origin().x, 5.0);
      EXPECT_DOUBLE_EQ(r.get_origin().y, 10.0);
      EXPECT_DOUBLE_EQ(r.get_origin().z, 15.0);
    }
  }

  // Verifica que el desplazamiento entre rayos de píxeles adyacentes es correcto, basándose en el
  // ancho de la imagen.
  TEST(CameraTest, PixelCenterDisplacement) {
    config cfg;
    cfg.set_image_width(100);
    cfg.set_aspect_ratio(1, 1);
    cfg.set_camera_position(vector(0, 0, 1));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    camera const cam(cfg);
    ray const r_00              = cam.get_ray(0.0, 0.0);
    ray const r_01              = cam.get_ray(0.01, 0.0);
    vector const d00            = r_00.get_direction();
    vector const d01            = r_01.get_direction();
    vector const delta          = d01 - d00;
    double const expected_delta = 0.02;
    EXPECT_NEAR(delta.magnitude(), expected_delta, 1e-3);
  }

  // Prueba una configuración de cámara no alineada con los ejes principales para asegurar que se
  // construye correctamente y el rayo central apunta al target.
  TEST(CameraTest, ArbitraryCameraOrientation) {
    config cfg;
    cfg.set_camera_position(vector(10, 5, 3));
    cfg.set_camera_target(vector(-2, 1, -4));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(75.0);
    cfg.set_aspect_ratio(4, 3);
    EXPECT_NO_THROW(camera const cam(cfg));
    camera const cam(cfg);
    ray const r = cam.get_ray(0.5, 0.5);
    EXPECT_DOUBLE_EQ(r.get_origin().x, 10.0);
    EXPECT_DOUBLE_EQ(r.get_origin().y, 5.0);
    EXPECT_DOUBLE_EQ(r.get_origin().z, 3.0);
    vector const to_target    = cfg.get_camera_target() - cfg.get_camera_position();
    vector const ray_dir      = r.get_direction().normalized();
    vector const expected_dir = to_target.normalized();
    EXPECT_NEAR(ray_dir.x, expected_dir.x, 1e-2);
    EXPECT_NEAR(ray_dir.y, expected_dir.y, 1e-2);
    EXPECT_NEAR(ray_dir.z, expected_dir.z, 1e-2);
  }

  // Verifica que cambiar el vector "norte" afecta la orientación de la cámara.
  TEST(CameraTest, NorthVectorAffectsOrientation) {
    config cfg1;
    cfg1.set_camera_position(vector(0, 0, 5));
    cfg1.set_camera_target(vector(0, 0, 0));
    cfg1.set_camera_north(vector(0, 1, 0));
    config cfg2;
    cfg2.set_camera_position(vector(0, 0, 5));
    cfg2.set_camera_target(vector(0, 0, 0));
    cfg2.set_camera_north(vector(1, 0, 0));
    camera const cam1(cfg1);
    camera const cam2(cfg2);
    ray const r1_top  = cam1.get_ray(0.5, 0.0);
    ray const r2_top  = cam2.get_ray(0.5, 0.0);
    vector const d1   = r1_top.get_direction().normalized();
    vector const d2   = r2_top.get_direction().normalized();
    double const diff = (d1 - d2).magnitude();
    EXPECT_GT(diff, 0.1);
  }

  // Prueba con un campo de visión muy estrecho para verificar la precisión del cálculo de rayos.
  TEST(CameraTest, VeryNarrowFOV) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 10));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(1.0);
    EXPECT_NO_THROW(camera const cam(cfg));
    camera const cam(cfg);
    ray const r_center     = cam.get_ray(0.5, 0.5);
    ray const r_corner     = cam.get_ray(0.0, 0.0);
    vector const d_center  = r_center.get_direction().normalized();
    vector const d_corner  = r_corner.get_direction().normalized();
    double const cos_angle = vector::dot(d_center, d_corner);
    EXPECT_GT(cos_angle, 0.999);
  }

  // Igual que el test anterior pero con un campo de visión muy amplio.
  TEST(CameraTest, WideAngleFOV) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 1));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(179.0);
    EXPECT_NO_THROW(camera const cam(cfg));
    camera const cam(cfg);
    ray const r_center     = cam.get_ray(0.5, 0.5);
    ray const r_top        = cam.get_ray(0.5, 1.0);
    vector const d_center  = r_center.get_direction().normalized();
    vector const d_top     = r_top.get_direction().normalized();
    double const cos_angle = vector::dot(d_center, d_top);
    double const angle_deg = std::acos(cos_angle) * 180.0 / std::numbers::pi;
    EXPECT_GT(angle_deg, 75.0);
    EXPECT_LT(angle_deg, 90.0);
  }

  // Prueba con la cámara muy cerca del objetivo.
  TEST(CameraTest, VeryCloseCameraToTarget) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 0.001));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    EXPECT_NO_THROW(camera const cam(cfg));
    camera const cam(cfg);
    ray const r      = cam.get_ray(0.5, 0.5);
    vector const dir = r.get_direction().normalized();
    EXPECT_NEAR(dir.z, -1.0, 1e-3);
  }

  // Esta vez estando la cámara muy lejos del objetivo.
  TEST(CameraTest, VeryFarCameraFromTarget) {
    config cfg;
    cfg.set_camera_position(vector(0, 0, 1'000));
    cfg.set_camera_target(vector(0, 0, 0));
    cfg.set_camera_north(vector(0, 1, 0));
    cfg.set_field_of_view(90.0);
    EXPECT_NO_THROW(camera const cam(cfg));
    camera const cam(cfg);
    ray const r      = cam.get_ray(0.5, 0.5);
    vector const dir = r.get_direction().normalized();
    EXPECT_NEAR(dir.z, -1.0, 1e-3);
  }

}  // namespace render
