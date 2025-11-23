#include "material.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

// Para crear archivos temporales
class TempSceneFile {
public:
  explicit TempSceneFile(std::string const & content, std::string const & suffix = "")
      : filename{"temp_scene_" + suffix + ".txt"} {
    std::ofstream ofs(filename);
    ofs << content;
  }

  ~TempSceneFile() { static_cast<void>(std::remove(filename.c_str())); }

  TempSceneFile(TempSceneFile const &)             = delete;
  TempSceneFile & operator=(TempSceneFile const &) = delete;
  TempSceneFile(TempSceneFile &&)                  = delete;
  TempSceneFile & operator=(TempSceneFile &&)      = delete;

  [[nodiscard]] std::string const & get_filename() const { return filename; }

private:
  std::string filename;
};

// Comprueba que parsear un archivo vacío no lanza excepciones.
TEST(SceneParserTest, ParseEmptyFile) {
  TempSceneFile const temp_file("");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Comprueba que parsear un archivo con solo espacios en blanco no lanza excepciones.
TEST(SceneParserTest, ParseFileWithOnlyWhitespace) {
  TempSceneFile const temp_file("   \n\n  \t  \n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Comprueba que se lanza una excepción si el archivo no existe.
TEST(SceneParserTest, FileNotFound) {
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file("nonexistent_file.txt", scn), std::runtime_error);
}

// Comprueba el parseo de una escena válida con un material y un objeto.
TEST(SceneParserTest, HandlesValidScene) {
  TempSceneFile const temp_file("matte: m1 0.8 0.1 0.1\n"
                                "sphere: 0 1 0 0.5 m1\n");

  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));

  render::material const * mat = scn.get_material("m1");
  ASSERT_NE(mat, nullptr);
  EXPECT_EQ(mat->get_type(), "matte");
}

TEST(SceneParserTest, ParseCompleteScene) {
  TempSceneFile const temp_file("matte: red 0.8 0.1 0.1\n"
                                "metal: silver 0.9 0.9 0.9 0.1\n"
                                "refractive: glass 1.5\n"
                                "sphere: 0 0 0 1.0 red\n"
                                "sphere: 2 0 0 0.5 silver\n"
                                "cylinder: 0 1 0 0.3 0 1 0 glass\n");

  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));

  EXPECT_NE(scn.get_material("red"), nullptr);
  EXPECT_NE(scn.get_material("silver"), nullptr);
  EXPECT_NE(scn.get_material("glass"), nullptr);
}

// Verifica que el parser maneja correctamente las etiquetas que incluyen dos puntos.
TEST(SceneParserTest, ParseWithColonSuffix) {
  TempSceneFile const temp_file("matte: m1 0.5 0.5 0.5\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
  EXPECT_NE(scn.get_material("m1"), nullptr);
}

// Verifica que el parser maneja correctamente las etiquetas que no incluyen dos puntos.
TEST(SceneParserTest, ParseWithoutColonSuffix) {
  TempSceneFile const temp_file("matte m1 0.5 0.5 0.5\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
  EXPECT_NE(scn.get_material("m1"), nullptr);
}

// Tests de Materiales: Matte

// Comprueba que se puede parsear un material mate correctamente.
TEST(SceneParserTest, ParseMatteMaterial) {
  TempSceneFile const temp_file("matte: red 1.0 0.0 0.0\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));

  render::material const * mat = scn.get_material("red");
  ASSERT_NE(mat, nullptr);
  EXPECT_EQ(mat->get_type(), "matte");
}

// Verifica que se lanza una excepción si faltan parámetros en la definición del material mate.
TEST(SceneParserTest, MatteInsufficientParameters) {
  TempSceneFile const temp_file("matte: m1 0.8 0.1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay parámetros extra en la definición del material mate.
TEST(SceneParserTest, MatteExtraParameters) {
  TempSceneFile const temp_file("matte: m1 0.8 0.1 0.1 extra\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si los valores de reflectancia son inválidos.
TEST(SceneParserTest, MatteInvalidReflectanceNegative) {
  TempSceneFile const temp_file("matte: m1 -0.1 0.5 0.5\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si los valores de reflectancia son mayores que 1.
TEST(SceneParserTest, MatteInvalidReflectanceOver1) {
  TempSceneFile const temp_file("matte: m1 0.5 1.5 0.5\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se puede parsear un material mate con valores de reflectancia en los límites
// válidos.
TEST(SceneParserTest, MatteValidReflectanceBoundary) {
  TempSceneFile const temp_file("matte: m1 0.0 0.5 1.0\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Tests de Materiales: Metal

// Verifica que se puede parsear un material metálico correctamente.
TEST(SceneParserTest, ParseMetalMaterial) {
  TempSceneFile const temp_file("metal: silver 0.9 0.9 0.9 0.1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));

  render::material const * mat = scn.get_material("silver");
  ASSERT_NE(mat, nullptr);
  EXPECT_EQ(mat->get_type(), "metal");
}

// Verifica que se lanza una excepción si faltan parámetros en la definición del material metálico.
TEST(SceneParserTest, MetalInsufficientParameters) {
  TempSceneFile const temp_file("metal: m1 0.9 0.9 0.9\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay parámetros extra en la definición del material
// metálico.
TEST(SceneParserTest, MetalExtraParameters) {
  TempSceneFile const temp_file("metal: m1 0.9 0.9 0.9 0.1 extra\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Comprueba que se lanza una excepción si los valores de reflectancia son inválidos.
TEST(SceneParserTest, MetalInvalidReflectance) {
  TempSceneFile const temp_file("metal: m1 1.5 0.5 0.5 0.1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Comprueba que se lanza una excepción si los valores de difusión son negativos.
TEST(SceneParserTest, MetalNegativeDiffusion) {
  TempSceneFile const temp_file("metal: m1 0.9 0.9 0.9 -0.1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Comprueba que se puede parsear un material metálico con difusión cero.
TEST(SceneParserTest, MetalZeroDiffusion) {
  TempSceneFile const temp_file("metal: m1 0.9 0.9 0.9 0.0\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Tests de Materiales: Refractive

// Verifica que se puede parsear un material refractivo correctamente.
TEST(SceneParserTest, ParseRefractiveMaterial) {
  TempSceneFile const temp_file("refractive: glass 1.5\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));

  render::material const * mat = scn.get_material("glass");
  ASSERT_NE(mat, nullptr);
  EXPECT_EQ(mat->get_type(), "refractive");
}

// Verifica que se lanza una excepción si faltan parámetros en la definición del material
// refractivo.
TEST(SceneParserTest, RefractiveInsufficientParameters) {
  TempSceneFile const temp_file("refractive: glass\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay parámetros extra en la definición del material
// refractivo.
TEST(SceneParserTest, RefractiveExtraParameters) {
  TempSceneFile const temp_file("refractive: glass 1.5 extra\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Comprueba que se lanza una excepción si el índice de refracción es negativo.
TEST(SceneParserTest, RefractiveNegativeIOR) {
  TempSceneFile const temp_file("refractive: glass -1.5\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Comprueba que se lanza una excepción si el índice de refracción es cero.
TEST(SceneParserTest, RefractiveZeroIOR) {
  TempSceneFile const temp_file("refractive: glass 0.0\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Comprueba que se puede parsear un material refractivo con un índice de refracción válido.
TEST(SceneParserTest, RefractiveValidIOR) {
  TempSceneFile const temp_file("refractive: water 1.333\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Tests de Nombres de Materiales Duplicados

// Verifica que se lanza una excepción si hay nombres de materiales duplicados.
TEST(SceneParserTest, ThrowsOnDuplicateMatteMaterialName) {
  TempSceneFile const temp_file("matte: duplicate 1.0 0.0 0.0\n"
                                "matte: duplicate 0.0 1.0 0.0\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay nombres de distintos materiales duplicados.
TEST(SceneParserTest, ThrowsOnDuplicateMixedMaterialName) {
  TempSceneFile const temp_file("matte: shared 1.0 0.0 0.0\n"
                                "metal: shared 0.0 1.0 0.0 0.5\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Tests de Objetos: Sphere

// Verifica que se puede parsear una esfera correctamente.
TEST(SceneParserTest, ParseSphere) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 1.0 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Verifica que se lanza una excepción si faltan parámetros en la definición de la esfera.
TEST(SceneParserTest, SphereInsufficientParameters) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 1.0\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay parámetros extra en la definición de la esfera.
TEST(SceneParserTest, SphereExtraParameters) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 1.0 m1 extra\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el material referenciado no está definido.
TEST(SceneParserTest, SphereUndefinedMaterial) {
  TempSceneFile const temp_file("sphere: 0 0 0 1.0 undefined\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el radio de la esfera es negativo.
TEST(SceneParserTest, SphereNegativeRadius) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 -1.0 m1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el radio de la esfera es cero.
TEST(SceneParserTest, SphereZeroRadius) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 0.0 m1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se puede parsear una esfera con un radio muy pequeño.
TEST(SceneParserTest, SphereVerySmallRadius) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 0.0001 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Tests de Objetos: Cylinder
// Verifica que se puede parsear un cilindro correctamente.
TEST(SceneParserTest, ParseCylinder) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "cylinder: 0 0 0 0.5 0 1 0 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Verifica que se lanza una excepción si faltan parámetros en la definición del cilindro.
TEST(SceneParserTest, CylinderInsufficientParameters) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "cylinder: 0 0 0 0.5 0 1 0\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay parámetros extra en la definición del cilindro.
TEST(SceneParserTest, CylinderExtraParameters) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "cylinder: 0 0 0 0.5 0 1 0 m1 extra\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el material referenciado no está definido.
TEST(SceneParserTest, CylinderUndefinedMaterial) {
  TempSceneFile const temp_file("cylinder: 0 0 0 0.5 0 1 0 undefined\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el radio del cilindro es negativo.
TEST(SceneParserTest, CylinderNegativeRadius) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "cylinder: 0 0 0 -0.5 0 1 0 m1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el radio del cilindro es cero.
TEST(SceneParserTest, CylinderZeroRadius) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "cylinder: 0 0 0 0.0 0 1 0 m1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si el vector de eje del cilindro es el vector cero.
TEST(SceneParserTest, CylinderZeroAxisVector) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "cylinder: 0 0 0 0.5 0 0 0 m1\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Tests de Etiquetas Desconocidas

// Verifica que se lanza una excepción si se encuentra una etiqueta desconocida.
TEST(SceneParserTest, ThrowsOnUnknownTag) {
  TempSceneFile const temp_file("triangle: 0 0 0\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Verifica que se lanza una excepción si hay un error tipográfico en la etiqueta.
TEST(SceneParserTest, ThrowsOnMisspelledTag) {
  TempSceneFile const temp_file("matt: m1 0.8 0.8 0.8\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Tests de Orden de Definiciones

// Verifica que se puede usar un material definido antes de su uso.
TEST(SceneParserTest, MaterialDefinedBeforeUse) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 1.0 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Verifica que se lanza una excepción si se usa un material antes de ser definido.
TEST(SceneParserTest, MaterialDefinedAfterUseFails) {
  TempSceneFile const temp_file("sphere: 0 0 0 1.0 m1\n"
                                "matte: m1 0.8 0.8 0.8\n");
  render::scene scn;
  EXPECT_THROW(render::parse_scene_file(temp_file.get_filename(), scn), std::runtime_error);
}

// Tests de Múltiples Materiales y Objetos
TEST(SceneParserTest, MultipleMaterialsAndObjects) {
  TempSceneFile const temp_file("matte: red 1.0 0.0 0.0\n"
                                "metal: silver 0.9 0.9 0.9 0.1\n"
                                "refractive: glass 1.5\n"
                                "sphere: 0 0 0 1.0 red\n"
                                "sphere: 2 0 0 0.5 silver\n"
                                "sphere: -2 0 0 0.5 glass\n"
                                "cylinder: 0 1 0 0.3 0 1 0 red\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));

  EXPECT_NE(scn.get_material("red"), nullptr);
  EXPECT_NE(scn.get_material("silver"), nullptr);
  EXPECT_NE(scn.get_material("glass"), nullptr);
}

// Tests de Valores Numéricos Edge Cases

// Verifica que se pueden parsear valores numéricos muy grandes.
TEST(SceneParserTest, ParseVeryLargeValues) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 1000 2000 3000 500.0 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Verifica que se pueden parsear valores numéricos muy pequeños pero positivos.
TEST(SceneParserTest, ParseVerySmallPositiveRadius) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: 0 0 0 0.00001 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Verifica que se pueden parsear coordenadas negativas.
TEST(SceneParserTest, ParseNegativeCoordinates) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "sphere: -10 -20 -30 1.0 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Verifica que se pueden parsear valores decimales con alta precisión.
TEST(SceneParserTest, ParseDecimalValues) {
  TempSceneFile const temp_file("matte: m1 0.123456 0.789012 0.345678\n"
                                "sphere: 0.5 1.5 2.5 0.75 m1\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
}

// Tests de Nombres de Materiales

// Verifica que se pueden usar nombres de materialeles con números.
TEST(SceneParserTest, MaterialNameWithNumbers) {
  TempSceneFile const temp_file("matte: mat123 0.8 0.8 0.8\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
  EXPECT_NE(scn.get_material("mat123"), nullptr);
}

// Verifica que se pueden usar nombres de materiales con guiones bajos.
TEST(SceneParserTest, MaterialNameWithUnderscore) {
  TempSceneFile const temp_file("matte: my_material_name 0.8 0.8 0.8\n");
  render::scene scn;
  ASSERT_NO_THROW(render::parse_scene_file(temp_file.get_filename(), scn));
  EXPECT_NE(scn.get_material("my_material_name"), nullptr);
}

// Tests de Información de Error

// Verifica que el mensaje de error contiene el número de línea donde ocurrió el error.
TEST(SceneParserTest, ErrorMessageContainsLineNumber) {
  TempSceneFile const temp_file("matte: m1 0.8 0.8 0.8\n"
                                "invalid_tag: something\n");
  render::scene scn;

  try {
    render::parse_scene_file(temp_file.get_filename(), scn);
    FAIL() << "Expected std::runtime_error";
  } catch (std::runtime_error const & e) {
    std::string const error_msg = e.what();
    EXPECT_NE(error_msg.find("line"), std::string::npos) << "Error message: " << error_msg;
    EXPECT_NE(error_msg.find('2'), std::string::npos);
  }
}

// Verifica que el mensaje de error contiene el contenido de la línea donde ocurrió el error.
TEST(SceneParserTest, ErrorMessageContainsLineContent) {
  TempSceneFile const temp_file("invalid_tag: something\n");
  render::scene scn;

  try {
    render::parse_scene_file(temp_file.get_filename(), scn);
    FAIL() << "Expected std::runtime_error";
  } catch (std::runtime_error const & e) {
    std::string const error_msg = e.what();
    EXPECT_NE(error_msg.find("invalid_tag"), std::string::npos) << "Error message: " << error_msg;
  }
}
