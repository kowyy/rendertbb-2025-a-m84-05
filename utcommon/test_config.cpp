#include "config.hpp"
#include "vector.hpp"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

namespace render {

  // Helper para crear archivos de configuración temporales
  class TempConfigFile {
  public:
    explicit TempConfigFile(std::string const & content) : filename{"temp_test_config.txt"} {
      std::ofstream ofs(filename);
      ofs << content;
    }

    ~TempConfigFile() {
      // Ignorar el valor de retorno intencionalmente
      static_cast<void>(std::remove(filename.c_str()));
    }

    // Eliminar copia
    TempConfigFile(TempConfigFile const &)             = delete;
    TempConfigFile & operator=(TempConfigFile const &) = delete;

    // Eliminar movimiento
    TempConfigFile(TempConfigFile &&)             = delete;
    TempConfigFile & operator=(TempConfigFile &&) = delete;

    [[nodiscard]] std::string const & get_filename() const { return filename; }

  private:
    mutable std::string filename;
  };

  // Pruebas de valores por defecto en config
  TEST(ConfigDefaultTest, AspectRatio) {
    config const cfg;
    EXPECT_EQ(cfg.get_aspect_width(), 16);
    EXPECT_EQ(cfg.get_aspect_height(), 9);
  }

  // Comprueba el valor por defecto del ancho de la imagen
  TEST(ConfigDefaultTest, ImageWidth) {
    config const cfg;
    EXPECT_EQ(cfg.get_image_width(), 1'920);
  }

  // Comprueba el valor por defecto de gamma
  TEST(ConfigDefaultTest, Gamma) {
    config const cfg;
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.2);
  }

  // Comprueba el valor por defecto de la posición de la cámara
  TEST(ConfigDefaultTest, CameraPosition) {
    config const cfg;
    vector const pos = cfg.get_camera_position();
    EXPECT_DOUBLE_EQ(pos.x, 0.0);
    EXPECT_DOUBLE_EQ(pos.y, 0.0);
    EXPECT_DOUBLE_EQ(pos.z, -10.0);
  }

  // Comprueba el valor por defecto del objetivo de la cámara
  TEST(ConfigDefaultTest, CameraTarget) {
    config const cfg;
    vector const target = cfg.get_camera_target();
    EXPECT_DOUBLE_EQ(target.x, 0.0);
    EXPECT_DOUBLE_EQ(target.y, 0.0);
    EXPECT_DOUBLE_EQ(target.z, 0.0);
  }

  // Comprueba el valor por defecto del vector "norte" de la cámara
  TEST(ConfigDefaultTest, CameraNorth) {
    config const cfg;
    vector const north = cfg.get_camera_north();
    EXPECT_DOUBLE_EQ(north.x, 0.0);
    EXPECT_DOUBLE_EQ(north.y, 1.0);
    EXPECT_DOUBLE_EQ(north.z, 0.0);
  }

  // Verifica el valor por defecto del campo de visión
  TEST(ConfigDefaultTest, FieldOfView) {
    config const cfg;
    EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 90.0);
  }

  // Comprueba el valor por defecto de las muestras por píxel
  TEST(ConfigDefaultTest, SamplesPerPixel) {
    config const cfg;
    EXPECT_EQ(cfg.get_samples_per_pixel(), 20);
  }

  // Comprueba el valor por defecto de la profundidad máxima de recursión
  TEST(ConfigDefaultTest, MaxDepth) {
    config const cfg;
    EXPECT_EQ(cfg.get_max_depth(), 5);
  }

  // Verifica el valor por defecto de la semilla del generador de números aleatorios para materiales
  TEST(ConfigDefaultTest, MaterialRngSeed) {
    config const cfg;
    EXPECT_EQ(cfg.get_material_rng_seed(), 13);
  }

  // Verifica el valor por defecto de la semilla del generador de números aleatorios para rayos
  TEST(ConfigDefaultTest, RayRngSeed) {
    config const cfg;
    EXPECT_EQ(cfg.get_ray_rng_seed(), 19);
  }

  // Verifica el color oscuro de fondo por defecto
  TEST(ConfigDefaultTest, BackgroundDarkColor) {
    config const cfg;
    vector const dark = cfg.get_background_dark_color();
    EXPECT_DOUBLE_EQ(dark.x, 0.25);
    EXPECT_DOUBLE_EQ(dark.y, 0.5);
    EXPECT_DOUBLE_EQ(dark.z, 1.0);
  }

  // Verifica el color claro de fondo por defecto
  TEST(ConfigDefaultTest, BackgroundLightColor) {
    config const cfg;
    vector const light = cfg.get_background_light_color();
    EXPECT_DOUBLE_EQ(light.x, 1.0);
    EXPECT_DOUBLE_EQ(light.y, 1.0);
    EXPECT_DOUBLE_EQ(light.z, 1.0);
  }

  // Comprueba la carga de 'aspect_ratio' desde un archivo.
  TEST(ConfigLoadTest, AspectRatio) {
    TempConfigFile const temp_file("aspect_ratio: 4 3\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_aspect_width(), 4);
    EXPECT_EQ(cfg.get_aspect_height(), 3);
  }

  // Comprueba la carga de 'image_width' desde un archivo.
  TEST(ConfigLoadTest, ImageWidth) {
    TempConfigFile const temp_file("image_width: 1280\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_image_width(), 1'280);
  }

  // Comprueba la carga de 'gamma' desde un archivo.
  TEST(ConfigLoadTest, Gamma) {
    TempConfigFile const temp_file("gamma: 2.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.0);
  }

  // Comprueba la carga de 'camera_position' desde un archivo.
  TEST(ConfigLoadTest, CameraPosition) {
    TempConfigFile const temp_file("camera_position: 10.0 20.0 30.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    vector const pos = cfg.get_camera_position();
    EXPECT_DOUBLE_EQ(pos.x, 10.0);
    EXPECT_DOUBLE_EQ(pos.y, 20.0);
    EXPECT_DOUBLE_EQ(pos.z, 30.0);
  }

  // Comprueba la carga de 'camera_target' desde un archivo.
  TEST(ConfigLoadTest, CameraTarget) {
    TempConfigFile const temp_file("camera_target: 5.0 5.0 5.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    vector const target = cfg.get_camera_target();
    EXPECT_DOUBLE_EQ(target.x, 5.0);
    EXPECT_DOUBLE_EQ(target.y, 5.0);
    EXPECT_DOUBLE_EQ(target.z, 5.0);
  }

  // Comprueba la carga de 'camera_north' desde un archivo.
  TEST(ConfigLoadTest, CameraNorth) {
    TempConfigFile const temp_file("camera_north: 0.0 0.0 1.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    vector const north = cfg.get_camera_north();
    EXPECT_DOUBLE_EQ(north.x, 0.0);
    EXPECT_DOUBLE_EQ(north.y, 0.0);
    EXPECT_DOUBLE_EQ(north.z, 1.0);
  }

  // Comprueba la carga de 'field_of_view' desde un archivo.
  TEST(ConfigLoadTest, FieldOfView) {
    TempConfigFile const temp_file("field_of_view: 60.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 60.0);
  }

  // Comprueba la carga de 'samples_per_pixel' desde un archivo.
  TEST(ConfigLoadTest, SamplesPerPixel) {
    TempConfigFile const temp_file("samples_per_pixel: 100\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_samples_per_pixel(), 100);
  }

  // Comprueba la carga de 'max_depth' desde un archivo.
  TEST(ConfigLoadTest, MaxDepth) {
    TempConfigFile const temp_file("max_depth: 10\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_max_depth(), 10);
  }

  // Comprueba la carga de 'material_rng_seed' desde un archivo.
  TEST(ConfigLoadTest, MaterialRngSeed) {
    TempConfigFile const temp_file("material_rng_seed: 42\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_material_rng_seed(), 42);
  }

  // Comprueba la carga de 'ray_rng_seed' desde un archivo.
  TEST(ConfigLoadTest, RayRngSeed) {
    TempConfigFile const temp_file("ray_rng_seed: 99\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_ray_rng_seed(), 99);
  }

  // Comprueba la carga de 'background_dark_color' desde un archivo.
  TEST(ConfigLoadTest, BackgroundDarkColor) {
    TempConfigFile const temp_file("background_dark_color: 0.1 0.2 0.3\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    vector const dark = cfg.get_background_dark_color();
    EXPECT_DOUBLE_EQ(dark.x, 0.1);
    EXPECT_DOUBLE_EQ(dark.y, 0.2);
    EXPECT_DOUBLE_EQ(dark.z, 0.3);
  }

  // Comprueba la carga de 'background_light_color' desde un archivo.
  TEST(ConfigLoadTest, BackgroundLightColor) {
    TempConfigFile const temp_file("background_light_color: 0.9 0.8 0.7\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    vector const light = cfg.get_background_light_color();
    EXPECT_DOUBLE_EQ(light.x, 0.9);
    EXPECT_DOUBLE_EQ(light.y, 0.8);
    EXPECT_DOUBLE_EQ(light.z, 0.7);
  }

  // Comprueba la carga de múltiples parámetros desde un archivo.
  TEST(ConfigLoadTest, MultipleParameters) {
    TempConfigFile const temp_file("image_width: 1280\n"
                                   "gamma: 2.0\n"
                                   "camera_position: 1.0 2.0 3.0\n"
                                   "max_depth: 7\n"
                                   "samples_per_pixel: 50\n");

    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_image_width(), 1'280);
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.0);
    EXPECT_EQ(cfg.get_max_depth(), 7);
    EXPECT_EQ(cfg.get_samples_per_pixel(), 50);
  }

  // Comprueba la carga de todos los parámetros desde un archivo.
  TEST(ConfigLoadTest, AllParameters) {
    TempConfigFile const temp_file("aspect_ratio: 21 9\n"
                                   "image_width: 2560\n"
                                   "gamma: 1.8\n"
                                   "camera_position: 5.0 10.0 15.0\n"
                                   "camera_target: 1.0 2.0 3.0\n"
                                   "camera_north: 0.0 1.0 0.0\n"
                                   "field_of_view: 75.0\n"
                                   "samples_per_pixel: 200\n"
                                   "max_depth: 15\n"
                                   "material_rng_seed: 123\n"
                                   "ray_rng_seed: 456\n"
                                   "background_dark_color: 0.2 0.4 0.6\n"
                                   "background_light_color: 0.8 0.9 1.0\n");

    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_aspect_width(), 21);
    EXPECT_EQ(cfg.get_aspect_height(), 9);
    EXPECT_EQ(cfg.get_image_width(), 2'560);
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 1.8);
    EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 75.0);
    EXPECT_EQ(cfg.get_samples_per_pixel(), 200);
    EXPECT_EQ(cfg.get_max_depth(), 15);
    EXPECT_EQ(cfg.get_material_rng_seed(), 123);
    EXPECT_EQ(cfg.get_ray_rng_seed(), 456);
  }

  // Comprueba que la carga falla si el ancho de 'aspect_ratio' es cero.
  TEST(ConfigValidationTest, AspectRatioZeroWidth) {
    TempConfigFile const temp_file("aspect_ratio: 0 9\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si la altura de 'aspect_ratio' es negativa.
  TEST(ConfigValidationTest, AspectRatioNegativeHeight) {
    TempConfigFile const temp_file("aspect_ratio: 16 -9\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si el ancho de la imagen es cero o negativo.
  TEST(ConfigValidationTest, ImageWidthZero) {
    TempConfigFile const temp_file("image_width: 0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si el ancho de la imagen es cero o negativo.
  TEST(ConfigValidationTest, ImageWidthNegative) {
    TempConfigFile const temp_file("image_width: -1920\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si gamma es cero o negativo.
  TEST(ConfigValidationTest, GammaZero) {
    TempConfigFile const temp_file("gamma: 0.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si gamma es cero o negativo.
  TEST(ConfigValidationTest, GammaNegative) {
    TempConfigFile const temp_file("gamma: -2.2\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si el campo de visión es cero o negativo.
  TEST(ConfigValidationTest, FieldOfViewZero) {
    TempConfigFile const temp_file("field_of_view: 0.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si el campo de visión es mayor o igual a 180.
  TEST(ConfigValidationTest, FieldOfView180) {
    TempConfigFile const temp_file("field_of_view: 180.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si el campo de visión es negativo.
  TEST(ConfigValidationTest, FieldOfViewNegative) {
    TempConfigFile const temp_file("field_of_view: -45.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si el campo de visión es mayor a 180.
  TEST(ConfigValidationTest, FieldOfViewOver180) {
    TempConfigFile const temp_file("field_of_view: 200.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si 'samples_per_pixel' es cero.
  TEST(ConfigValidationTest, SamplesPerPixelZero) {
    TempConfigFile const temp_file("samples_per_pixel: 0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si 'samples_per_pixel' es negativo.
  TEST(ConfigValidationTest, SamplesPerPixelNegative) {
    TempConfigFile const temp_file("samples_per_pixel: -20\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si 'max_depth' es cero.
  TEST(ConfigValidationTest, MaxDepthZero) {
    TempConfigFile const temp_file("max_depth: 0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Comprueba que la carga falla si 'max_depth' es negativo.
  TEST(ConfigValidationTest, MaxDepthNegative) {
    TempConfigFile const temp_file("max_depth: -5\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si algún componente del color oscuro de fondo es menor a 0 o mayor
  // a 1.
  TEST(ConfigValidationTest, BackgroundDarkColorRedNegative) {
    TempConfigFile const temp_file("background_dark_color: -0.1 0.5 0.5\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si algún componente del color oscuro de fondo es menor a 0 o mayor
  // a 1.
  TEST(ConfigValidationTest, BackgroundDarkColorGreenOver1) {
    TempConfigFile const temp_file("background_dark_color: 0.5 1.5 0.5\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si algún componente del color claro de fondo es menor a 0 o mayor
  // a 1.
  TEST(ConfigValidationTest, BackgroundLightColorBlueNegative) {
    TempConfigFile const temp_file("background_light_color: 0.5 0.5 -0.1\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si algún componente del color claro de fondo es menor a 0 o mayor
  // a 1.
  TEST(ConfigValidationTest, BackgroundLightColorAllOver1) {
    TempConfigFile const temp_file("background_light_color: 2.0 2.0 2.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  //
  TEST(ConfigParsingTest, UnknownKey) {
    TempConfigFile const temp_file("unknown_parameter: 123\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si 'aspect_ratio' tiene menos de dos argumentos.
  TEST(ConfigParsingTest, AspectRatioInsufficientArgs) {
    TempConfigFile const temp_file("aspect_ratio: 16\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si 'image_width' no tiene argumentos.
  TEST(ConfigParsingTest, ImageWidthNoArgs) {
    TempConfigFile const temp_file("image_width:\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si 'camera_position' tiene menos de tres argumentos.
  TEST(ConfigParsingTest, CameraPositionInsufficientArgs) {
    TempConfigFile const temp_file("camera_position: 1.0 2.0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si 'background_dark_color' tiene menos de tres argumentos.
  TEST(ConfigParsingTest, BackgroundColorInsufficientArgs) {
    TempConfigFile const temp_file("background_dark_color: 0.5 0.5\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Verifica que la carga falla si 'file_path' no existe.
  TEST(ConfigParsingTest, FileNotFound) {
    config cfg;
    EXPECT_THROW(load_config("this_file_does_not_exist.txt", cfg), std::runtime_error);
  }

  // Comprueba el análisis de un archivo vacío.
  TEST(ConfigParsingTest, EmptyFile) {
    TempConfigFile const temp_file("");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_image_width(), 1'920);
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.2);
  }

  // Comprueba el análisis de una línea con solo espacios en blanco.
  TEST(ConfigParsingTest, OnlyWhitespace) {
    TempConfigFile const temp_file("   \n\n  \t  \n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_max_depth(), 5);
  }

  // Verifica el análisis cuando hay espacios adicionales alrededor de los dos puntos.
  TEST(ConfigParsingTest, WithColonSuffix) {
    TempConfigFile const temp_file("image_width: 1280\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_image_width(), 1'280);
  }

  // Comprueba que
  TEST(ConfigEdgeCaseTest, VeryLargeImageWidth) {
    TempConfigFile const temp_file("image_width: 10000\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_image_width(), 10'000);
  }

  // Verifica que la carga falla si 'gamma' es menor a 1.
  TEST(ConfigEdgeCaseTest, VerySmallGamma) {
    TempConfigFile const temp_file("gamma: 0.1\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 0.1);
  }

  // Verifica que la carga falla si 'field_of_view' es menor a 0.
  TEST(ConfigEdgeCaseTest, FieldOfViewMinimum) {
    TempConfigFile const temp_file("field_of_view: 0.001\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_NEAR(cfg.get_field_of_view(), 0.001, 1e-6);
  }

  // Comprueba que la carga falla si 'field_of_view' es mayor a 180.
  TEST(ConfigEdgeCaseTest, FieldOfViewMaximum) {
    TempConfigFile const temp_file("field_of_view: 179.999\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_NEAR(cfg.get_field_of_view(), 179.999, 1e-6);
  }

  // Comprueba que se puede cargar un color de fondo negro (0,0,0).
  TEST(ConfigEdgeCaseTest, BackgroundColorAllZeros) {
    TempConfigFile const temp_file("background_dark_color: 0.0 0.0 0.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));

    vector const dark = cfg.get_background_dark_color();
    EXPECT_DOUBLE_EQ(dark.x, 0.0);
    EXPECT_DOUBLE_EQ(dark.y, 0.0);
    EXPECT_DOUBLE_EQ(dark.z, 0.0);
  }

  // Verifica que se puede cargar un color de fondo blanco (1,1,1).
  TEST(ConfigEdgeCaseTest, BackgroundColorAllOnes) {
    TempConfigFile const temp_file("background_light_color: 1.0 1.0 1.0\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));

    vector const light = cfg.get_background_light_color();
    EXPECT_DOUBLE_EQ(light.x, 1.0);
    EXPECT_DOUBLE_EQ(light.y, 1.0);
    EXPECT_DOUBLE_EQ(light.z, 1.0);
  }

  // Tests para nuevos parámetros TBB

  // Pruebas de valores por defecto para parámetros TBB
  TEST(ConfigDefaultTest, NumThreads) {
    config const cfg;
    EXPECT_EQ(cfg.get_num_threads(), -1);
  }

  TEST(ConfigDefaultTest, GrainSize) {
    config const cfg;
    EXPECT_EQ(cfg.get_grain_size(), 1);
  }

  TEST(ConfigDefaultTest, Partitioner) {
    config const cfg;
    EXPECT_EQ(cfg.get_partitioner(), "auto");
  }

  // Pruebas de carga desde archivo para parámetros TBB
  TEST(ConfigLoadTest, NumThreadsAuto) {
    TempConfigFile const temp_file("num_threads: -1\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_num_threads(), -1);
  }

  TEST(ConfigLoadTest, NumThreadsSpecific) {
    TempConfigFile const temp_file("num_threads: 8\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_num_threads(), 8);
  }

  TEST(ConfigLoadTest, GrainSize) {
    TempConfigFile const temp_file("grain_size: 50\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_grain_size(), 50);
  }

  TEST(ConfigLoadTest, PartitionerAuto) {
    TempConfigFile const temp_file("partitioner: auto\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_partitioner(), "auto");
  }

  TEST(ConfigLoadTest, PartitionerSimple) {
    TempConfigFile const temp_file("partitioner: simple\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_partitioner(), "simple");
  }

  TEST(ConfigLoadTest, PartitionerStatic) {
    TempConfigFile const temp_file("partitioner: static\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_partitioner(), "static");
  }

  // Pruebas de validación para parámetros TBB
  TEST(ConfigValidationTest, NumThreadsZero) {
    TempConfigFile const temp_file("num_threads: 0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigValidationTest, NumThreadsInvalidNegative) {
    TempConfigFile const temp_file("num_threads: -5\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigValidationTest, GrainSizeZero) {
    TempConfigFile const temp_file("grain_size: 0\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigValidationTest, GrainSizeNegative) {
    TempConfigFile const temp_file("grain_size: -10\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigValidationTest, PartitionerInvalid) {
    TempConfigFile const temp_file("partitioner: dynamic\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigValidationTest, PartitionerInvalidRandom) {
    TempConfigFile const temp_file("partitioner: random_string\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Prueba de parsing para parámetros TBB
  TEST(ConfigParsingTest, NumThreadsNoArgs) {
    TempConfigFile const temp_file("num_threads:\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigParsingTest, GrainSizeNoArgs) {
    TempConfigFile const temp_file("grain_size:\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  TEST(ConfigParsingTest, PartitionerNoArgs) {
    TempConfigFile const temp_file("partitioner:\n");
    config cfg;
    EXPECT_THROW(load_config(temp_file.get_filename(), cfg), std::runtime_error);
  }

  // Pruebas de casos límite para parámetros TBB
  TEST(ConfigEdgeCaseTest, NumThreadsMaximum) {
    TempConfigFile const temp_file("num_threads: 256\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_num_threads(), 256);
  }

  TEST(ConfigEdgeCaseTest, GrainSizeMinimum) {
    TempConfigFile const temp_file("grain_size: 1\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_grain_size(), 1);
  }

  TEST(ConfigEdgeCaseTest, GrainSizeLarge) {
    TempConfigFile const temp_file("grain_size: 1000\n");
    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_grain_size(), 1'000);
  }

  // Prueba de configuración completa incluyendo parámetros TBB
  TEST(ConfigLoadTest, AllParametersWithTBB) {
    TempConfigFile const temp_file("aspect_ratio: 21 9\n"
                                   "image_width: 2560\n"
                                   "gamma: 1.8\n"
                                   "camera_position: 5.0 10.0 15.0\n"
                                   "camera_target: 1.0 2.0 3.0\n"
                                   "camera_north: 0.0 1.0 0.0\n"
                                   "field_of_view: 75.0\n"
                                   "samples_per_pixel: 200\n"
                                   "max_depth: 15\n"
                                   "material_rng_seed: 123\n"
                                   "ray_rng_seed: 456\n"
                                   "num_threads: 16\n"
                                   "grain_size: 10\n"
                                   "partitioner: static\n"
                                   "background_dark_color: 0.2 0.4 0.6\n"
                                   "background_light_color: 0.8 0.9 1.0\n");

    config cfg;
    ASSERT_NO_THROW(load_config(temp_file.get_filename(), cfg));
    EXPECT_EQ(cfg.get_aspect_width(), 21);
    EXPECT_EQ(cfg.get_aspect_height(), 9);
    EXPECT_EQ(cfg.get_image_width(), 2'560);
    EXPECT_DOUBLE_EQ(cfg.get_gamma(), 1.8);
    EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 75.0);
    EXPECT_EQ(cfg.get_samples_per_pixel(), 200);
    EXPECT_EQ(cfg.get_max_depth(), 15);
    EXPECT_EQ(cfg.get_material_rng_seed(), 123);
    EXPECT_EQ(cfg.get_ray_rng_seed(), 456);
    EXPECT_EQ(cfg.get_num_threads(), 16);
    EXPECT_EQ(cfg.get_grain_size(), 10);
    EXPECT_EQ(cfg.get_partitioner(), "static");
  }

}  // namespace render
