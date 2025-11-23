#include "scene_parser.hpp"
#include "material.hpp"
#include "object.hpp"
#include "scene.hpp"
#include "vector.hpp"
#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

  // Divide una cadena en tokens separados por espacios
  std::vector<std::string> split_ws(std::string const & s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) {
      tokens.push_back(tok);
    }
    return tokens;
  }

  // Parsea un vector 3D desde tres tokens consecutivos
  render::vector parse_vector(std::vector<std::string> const & parts, size_t start_index) {
    if (start_index + 2 >= parts.size()) {
      throw std::runtime_error("Insufficient vector components");
    }
    return render::vector{std::stod(parts[start_index]), std::stod(parts[start_index + 1]),
                          std::stod(parts[start_index + 2])};
  }

  // Verifica que todos los componentes de reflectancia estén en [0, 1]
  void validate_reflectance(render::vector const & reflectance, std::string const & material_type,
                            std::string const & line) {
    if (reflectance.x < 0.0 or
        reflectance.x > 1.0 or
        reflectance.y < 0.0 or
        reflectance.y > 1.0 or
        reflectance.z < 0.0 or
        reflectance.z > 1.0)
    {
      throw std::runtime_error(
          "Error: Invalid " + material_type + " material parameters\nLine: " + line);
    }
  }

  // Verifica que el número de parámetros sea exactamente el esperado
  void check_exact_size(std::vector<std::string> const & parts, size_t expected,
                        std::string const & entity_type, std::string const & line) {
    if (parts.size() < expected) {
      throw std::runtime_error("Error: Invalid " + entity_type + " parameters\nLine: " + line);
    }

    if (parts.size() > expected) {
      std::string extra;
      for (size_t i = expected; i < parts.size(); ++i) {
        if (i > expected) {
          extra += " ";
        }
        extra += parts[i];
      }

      throw std::runtime_error("Error: Extra data after configuration value for key " +
                               entity_type +
                               "\nExtra: " +
                               extra +
                               "\nLine: " +
                               line);
    }
  }

  // Verifica que el nombre del material sea único
  void validate_material_unique(render::scene const & scn, std::string const & name,
                                std::string const & line) {
    if (scn.get_material(name) != nullptr) {
      throw std::runtime_error(
          "Error: Material with name [" + name + "] already exists\nLine: " + line);
    }
  }

  // PARSEADORES DE MATERIALES

  void parse_matte(std::vector<std::string> const & parts, std::string const & line,
                   render::scene & scn) {
    check_exact_size(parts, 5, "matte", line);

    std::string const & name = parts[1];
    validate_material_unique(scn, name, line);

    auto const reflectance = parse_vector(parts, 2);
    validate_reflectance(reflectance, "matte", line);

    scn.add_material(name, std::make_unique<render::matte_material>(reflectance));
  }

  void parse_metal(std::vector<std::string> const & parts, std::string const & line,
                   render::scene & scn) {
    check_exact_size(parts, 6, "metal", line);

    std::string const & name = parts[1];
    validate_material_unique(scn, name, line);

    auto const reflectance = parse_vector(parts, 2);
    validate_reflectance(reflectance, "metal", line);

    double const diffusion = std::stod(parts[5]);
    if (diffusion < 0.0) {
      throw std::runtime_error("Error: Invalid metal material parameters\nLine: " + line);
    }

    scn.add_material(name, std::make_unique<render::metal_material>(reflectance, diffusion));
  }

  void parse_refractive(std::vector<std::string> const & parts, std::string const & line,
                        render::scene & scn) {
    check_exact_size(parts, 3, "refractive", line);

    std::string const & name = parts[1];
    validate_material_unique(scn, name, line);

    double const ior = std::stod(parts[2]);
    if (ior <= 0.0) {
      throw std::runtime_error("Error: Invalid refractive material parameters\nLine: " + line);
    }

    scn.add_material(name, std::make_unique<render::refractive_material>(ior));
  }

  // PARSEADORES DE OBJETOS

  void parse_sphere(std::vector<std::string> const & parts, std::string const & line,
                    render::scene & scn) {
    check_exact_size(parts, 6, "sphere", line);

    auto const center            = parse_vector(parts, 1);
    double const radius          = std::stod(parts[4]);
    std::string const & mat_name = parts[5];

    if (radius <= 0.0) {
      throw std::runtime_error("Error: Invalid sphere parameters\nLine: " + line);
    }

    render::material const * mat = scn.get_material(mat_name);
    if (mat == nullptr) {
      throw std::runtime_error("Error: Material not found [" + mat_name + "]\nLine: " + line);
    }

    scn.add_object(std::make_unique<render::sphere>(center, radius, mat));
  }

  void parse_cylinder(std::vector<std::string> const & parts, std::string const & line,
                      render::scene & scn) {
    check_exact_size(parts, 9, "cylinder", line);

    auto const center            = parse_vector(parts, 1);
    double const radius          = std::stod(parts[4]);
    auto const axis              = parse_vector(parts, 5);
    std::string const & mat_name = parts[8];

    if (radius <= 0.0) {
      throw std::runtime_error("Error: Invalid cylinder parameters\nLine: " + line);
    }

    if (axis.is_near_zero()) {
      throw std::runtime_error("Error: Invalid cylinder parameters\nLine: " + line);
    }

    render::material const * mat = scn.get_material(mat_name);
    if (mat == nullptr) {
      throw std::runtime_error("Error: Material not found [" + mat_name + "]\nLine: " + line);
    }

    scn.add_object(std::make_unique<render::cylinder>(center, radius, axis, mat));
  }

}  // namespace

namespace render {

  // Lee archivo de escena línea por línea y parsea materiales y objetos
  void parse_scene_file(std::string const & path, scene & scn) {
    std::ifstream ifs(path);
    if (!ifs) {
      throw std::runtime_error("Error: Cannot open scene file: " + path);
    }
    std::string line;
    int line_number = 0;
    while (std::getline(ifs, line)) {
      line_number++;
      auto const parts = split_ws(line);
      if (parts.empty()) {
        continue;
      }

      std::string tag = parts[0];

      // Eliminar ':' del final si existe
      if (!tag.empty() and tag.back() == ':') {
        tag.pop_back();
      }

      // Parsear según tipo de entidad
      if (tag == "matte") {
        parse_matte(parts, line, scn);
      } else if (tag == "metal") {
        parse_metal(parts, line, scn);
      } else if (tag == "refractive") {
        parse_refractive(parts, line, scn);
      } else if (tag == "sphere") {
        parse_sphere(parts, line, scn);
      } else if (tag == "cylinder") {
        parse_cylinder(parts, line, scn);
      } else {
        throw std::runtime_error("Error on line " +
                                 std::to_string(line_number) +
                                 ": Unknown scene entity [" +
                                 tag +
                                 "]");
      }
    }
  }

}  // namespace render
