#include "application.hpp"
#include "color.hpp"
#include "image_soa.hpp"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>

class ApplicationTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::ofstream config_file(config_filename);
    config_file << "image_width: 100\n";
    config_file << "camera_position: 0 0 -1\n";
    config_file.close();

    std::ofstream scene_file(scene_filename);
    scene_file << "matte: mat1 0.8 0.8 0.8\n";
    scene_file << "sphere: 0 0 0 0.5 mat1\n";
    scene_file.close();
  }

  void TearDown() override {
    std::filesystem::remove(config_filename);
    std::filesystem::remove(scene_filename);
    std::filesystem::remove(output_filename);
  }

  std::string const config_filename = "temp_test_config.txt";
  std::string const scene_filename  = "temp_test_scene.txt";
  std::string const output_filename = "temp_test_output.ppm";
};

// Comprueba que la aplicación falla si se ejecuta sin argumentos.
TEST(ApplicationArgsTest, FailsWithNoArguments) {
  std::array argv = {"render-soa"};
  auto result     = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

// Verifica si la aplicación falla cuando se ejecuta con un número incorrecto de argumentos
TEST(ApplicationArgsTest, FailsWithIncorrectArgumentCount) {
  std::array argv = {"render-soa", "config.txt", "scene.txt"};
  auto result     = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

// Comprueba una ejecución exitosa de la aplicación con argumentos válidos.
TEST_F(ApplicationTest, SuccessfulRun) {
  std::array argv = {"render-soa", config_filename.c_str(), scene_filename.c_str(),
                     output_filename.c_str()};
  auto result     = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_SUCCESS);
  ASSERT_TRUE(std::filesystem::exists(output_filename));
}

// Verifica si la aplicación falla si no se encuentra el archivo de configuración.
TEST_F(ApplicationTest, FailsWhenConfigFileIsNotFound) {
  std::array argv   = {"render-soa", "archivo_que_no_existe.txt", scene_filename.c_str(),
                       output_filename.c_str()};
  auto const result = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

// Prueba que la aplicación falla si no se encuentra el archivo de escena.
TEST_F(ApplicationTest, FailsWhenSceneFileIsNotFound) {
  std::array argv   = {"render-soa", config_filename.c_str(), "archivo_que_no_existe.txt",
                       output_filename.c_str()};
  auto const result = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

namespace {

  std::string readFile(std::string const & filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Test helper could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

}  // anonymous namespace

class ImageSOATest : public ::testing::Test {
protected:
  void TearDown() override { std::filesystem::remove(test_filename); }

  std::string const test_filename = "temp_image_soa_test.ppm";
};

// Verifica que el constructor de ImageSOA almacena correctamente el ancho y alto, y que get_width()
// y get_height() devuelven esos valores.
TEST(ImageSOA, ConstructorAndGetters) {
  ImageSOA const image(100, 200);
  EXPECT_EQ(image.get_width(), 100);
  EXPECT_EQ(image.get_height(), 200);
}

// Prueba del Constructor con tamaños inválidos (0 o negativos)
TEST(ImageSOA, ConstructorHandlesInvalidSizes) {
  ImageSOA const img_zero_width(0, 100);
  EXPECT_EQ(img_zero_width.get_width(), 0);
  EXPECT_EQ(img_zero_width.get_height(), 100);

  ImageSOA const img_negative_dims(-10, -20);
  EXPECT_EQ(img_negative_dims.get_width(), -10);
  EXPECT_EQ(img_negative_dims.get_height(), -20);
}

// Comprueba si set_pixel y save_ppm escriben el contenido correcto
TEST_F(ImageSOATest, SetPixelAndSavePPM) {
  ImageSOA image(2, 1);
  double const gamma = 1.0;
  image.set_pixel(0, 0, render::color{1.0, 0.0, 0.0}, gamma);
  image.set_pixel(1, 0, render::color{0.0, 0.0, 1.0}, gamma);
  image.save_ppm(test_filename);
  ASSERT_TRUE(std::filesystem::exists(test_filename));
  std::string const expected_content = "P3\n"
                                       "2 1\n"
                                       "255\n"
                                       "255 0 0\n"
                                       "0 0 255\n";
  std::string const actual_content   = readFile(test_filename);
  EXPECT_EQ(actual_content, expected_content);
}

// Comprueba si save_ppm falla con una ruta de archivo inválida
TEST(ImageSOA, SavePPMThrowsOnInvalidPath) {
  ImageSOA const image(1, 1);
  std::string const invalid_path = "/non_existent_directory/test.ppm";
  ASSERT_THROW(image.save_ppm(invalid_path), std::runtime_error);
}

// Verifica que set_pixel falla cuando coordenadas están fuera de rango
TEST(ImageSOA, SetPixelThrowsOnOutOfBounds) {
  ImageSOA image(10, 10);
  double const gamma = 1.0;
  render::color const black{0.0, 0.0, 0.0};
  ASSERT_NO_THROW(image.set_pixel(9, 9, black, gamma));
  ASSERT_THROW(image.set_pixel(10, 9, black, gamma), std::out_of_range);
  ASSERT_THROW(image.set_pixel(9, 10, black, gamma), std::out_of_range);
}
