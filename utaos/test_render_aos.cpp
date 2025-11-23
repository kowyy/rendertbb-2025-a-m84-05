#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

#include "application.hpp"

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

// Prueba que la aplicación falla si se ejecuta sin argumentos.
TEST(ApplicationArgsTest, FailsWithNoArguments) {
  std::array argv = {"render-aos"};
  auto result     = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

// Verifica que la aplicación falla si se ejecuta con un número incorrecto de argumentos
TEST(ApplicationArgsTest, FailsWithIncorrectArgumentCount) {
  std::array argv = {"render-aos", "config.txt", "scene.txt"};
  auto result     = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

// Comprueba una ejecución exitosa de la aplicación utilizando los archivos temporales creados.
TEST_F(ApplicationTest, SuccessfulRun) {
  std::array argv = {"render-aos", config_filename.c_str(), scene_filename.c_str(),
                     output_filename.c_str()};

  auto result = render::Application::run(argv);

  ASSERT_EQ(result, EXIT_SUCCESS);
  ASSERT_TRUE(std::filesystem::exists(output_filename));
}

// Comprueba si la aplicación falla si no se encuentra el archivo de configuración.
TEST_F(ApplicationTest, FailsWhenConfigFileIsNotFound) {
  std::array argv   = {"render-aos", "archivo_que_no_existe.txt", scene_filename.c_str(),
                       output_filename.c_str()};
  auto const result = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}

// Verifica si la aplicación falla si no se encuentra el archivo de escena.
TEST_F(ApplicationTest, FailsWhenSceneFileIsNotFound) {
  std::array argv   = {"render-aos", config_filename.c_str(), "archivo_que_no_existe.txt",
                       output_filename.c_str()};
  auto const result = render::Application::run(argv);
  ASSERT_EQ(result, EXIT_FAILURE);
}
