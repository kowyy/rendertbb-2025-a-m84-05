#include "config.hpp"
#include "vector.hpp"
#include <cstdint>
#include <fstream>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace render {

  namespace {

    // Elimina espacios en blanco al inicio y final de una cadena
    inline std::string trim_copy(std::string const & s) {
      auto const begin = s.find_first_not_of(" \t\r\n");
      if (begin == std::string::npos) {
        return {};
      }
      auto const end = s.find_last_not_of(" \t\r\n");
      return s.substr(begin, end - begin + 1);
    }

    // Divide cadena en tokens separados por espacios
    inline std::vector<std::string> split_ws(std::string const & s) {
      std::istringstream iss(s);
      std::vector<std::string> tokens;
      std::string tok;
      while (iss >> tok) {
        tokens.push_back(tok);
      }
      return tokens;
    }

    // Conversiones de string a tipos numéricos
    inline int to_int(std::string const & s) {
      return std::stoi(s);
    }

    inline double to_double(std::string const & s) {
      return std::stod(s);
    }

    inline std::uint64_t to_u64(std::string const & s) {
      return std::stoull(s);
    }

    // Handlers para cada clave de configuración
    void handle_aspect_ratio(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 3) {
        throw std::runtime_error("Error: Invalid value for key: [aspect_ratio:]");
      }
      int const width  = to_int(parts[1]);
      int const height = to_int(parts[2]);
      cfg.set_aspect_ratio(width, height);
    }

    void handle_image_width(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [image_width:]");
      }
      int const width = to_int(parts[1]);
      cfg.set_image_width(width);
    }

    void handle_gamma(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [gamma:]");
      }
      double const g = to_double(parts[1]);
      cfg.set_gamma(g);
    }

    void handle_camera_position(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 4) {
        throw std::runtime_error("Error: Invalid value for key: [camera_position:]");
      }
      vector const pos{to_double(parts[1]), to_double(parts[2]), to_double(parts[3])};
      cfg.set_camera_position(pos);
    }

    void handle_camera_target(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 4) {
        throw std::runtime_error("Error: Invalid value for key: [camera_target:]");
      }
      vector const target{to_double(parts[1]), to_double(parts[2]), to_double(parts[3])};
      cfg.set_camera_target(target);
    }

    void handle_camera_north(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 4) {
        throw std::runtime_error("Error: Invalid value for key: [camera_north:]");
      }
      vector const north{to_double(parts[1]), to_double(parts[2]), to_double(parts[3])};
      cfg.set_camera_north(north);
    }

    void handle_field_of_view(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [field_of_view:]");
      }
      double const fov = to_double(parts[1]);
      cfg.set_field_of_view(fov);
    }

    void handle_samples_per_pixel(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [samples_per_pixel:]");
      }
      int const samples = to_int(parts[1]);
      cfg.set_samples_per_pixel(samples);
    }

    void handle_max_depth(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [max_depth:]");
      }
      int const depth = to_int(parts[1]);
      cfg.set_max_depth(depth);
    }

    void handle_material_rng_seed(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [material_rng_seed:]");
      }
      std::uint64_t const seed = to_u64(parts[1]);
      cfg.set_material_rng_seed(seed);
    }

    void handle_ray_rng_seed(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 2) {
        throw std::runtime_error("Error: Invalid value for key: [ray_rng_seed:]");
      }
      std::uint64_t const seed = to_u64(parts[1]);
      cfg.set_ray_rng_seed(seed);
    }

    void handle_background_dark_color(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 4) {
        throw std::runtime_error("Error: Invalid value for key: [background_dark_color:]");
      }
      double const r = to_double(parts[1]);
      double const g = to_double(parts[2]);
      double const b = to_double(parts[3]);
      vector const color{r, g, b};
      cfg.set_background_dark_color(color);
    }

    void handle_background_light_color(std::vector<std::string> const & parts, config & cfg) {
      if (parts.size() != 4) {
        throw std::runtime_error("Error: Invalid value for key: [background_light_color:]");
      }
      double const r = to_double(parts[1]);
      double const g = to_double(parts[2]);
      double const b = to_double(parts[3]);
      vector const color{r, g, b};
      cfg.set_background_light_color(color);
    }

    // Procesa todas las líneas del archivo de configuración
    void process_lines(std::istream & in,
                       std::unordered_map<std::string, void (*)(std::vector<std::string> const &,
                                                                config &)> const & handlers,
                       config & out) {
      std::string line;
      while (std::getline(in, line)) {
        auto const trimmed = trim_copy(line);
        if (trimmed.empty()) {
          continue;
        }

        auto parts = split_ws(trimmed);
        if (parts.empty()) {
          continue;
        }

        // Eliminar ':' del final de la clave si existe
        std::string key = parts[0];
        if (!key.empty() and key.back() == ':') {
          key.pop_back();
        }

        auto it = handlers.find(key);
        if (it == handlers.end()) {
          throw std::runtime_error("Error: Unknown configuration key: [" + key + ":]");
        }
        it->second(parts, out);
      }
    }

  }  // namespace

  // SETTERS CON VALIDACIÓN

  void config::set_aspect_ratio(int const width, int const height) {
    if (width <= 0 or height <= 0) {
      throw std::runtime_error("Error: Invalid value for key: [aspect_ratio:]");
    }
    aspect_w = width;
    aspect_h = height;
  }

  void config::set_image_width(int const width) {
    if (width <= 0) {
      throw std::runtime_error("Error: Invalid value for key: [image_width:]");
    }
    image_width = width;
  }

  void config::set_gamma(double const g) {
    if (g <= 0.0) {
      throw std::runtime_error("Error: Invalid value for key: [gamma:]");
    }
    gamma = g;
  }

  void config::set_camera_position(vector const & pos) {
    camera_position = pos;
  }

  void config::set_camera_target(vector const & target) {
    camera_target = target;
  }

  void config::set_camera_north(vector const & north) {
    if (north.is_near_zero()) {
      throw std::runtime_error("Error: Invalid value for key: [camera_north:]");
    }
    camera_north = north;
  }

  void config::set_field_of_view(double const fov) {
    if (fov <= 0.0 or fov >= 180.0) {
      throw std::runtime_error("Error: Invalid value for key: [field_of_view:]");
    }
    field_of_view = fov;
  }

  void config::set_samples_per_pixel(int const samples) {
    if (samples <= 0) {
      throw std::runtime_error("Error: Invalid value for key: [samples_per_pixel:]");
    }
    samples_per_pixel = samples;
  }

  void config::set_max_depth(int const depth) {
    if (depth <= 0) {
      throw std::runtime_error("Error: Invalid value for key: [max_depth:]");
    }
    max_depth = depth;
  }

  void config::set_material_rng_seed(std::uint64_t const seed) {
    if (seed == 0) {
      throw std::runtime_error("Error: Invalid value for key: [material_rng_seed:]");
    }
    material_rng_seed = seed;
  }

  void config::set_ray_rng_seed(std::uint64_t const seed) {
    if (seed == 0) {
      throw std::runtime_error("Error: Invalid value for key: [ray_rng_seed:]");
    }
    ray_rng_seed = seed;
  }

  void config::set_background_dark_color(vector const & color) {
    if (color.x < 0.0 or
        color.x > 1.0 or
        color.y < 0.0 or
        color.y > 1.0 or
        color.z < 0.0 or
        color.z > 1.0)
    {
      throw std::runtime_error("Error: Invalid value for key: [background_dark_color:]");
    }
    background_dark_color = color;
  }

  void config::set_background_light_color(vector const & color) {
    if (color.x < 0.0 or
        color.x > 1.0 or
        color.y < 0.0 or
        color.y > 1.0 or
        color.z < 0.0 or
        color.z > 1.0)
    {
      throw std::runtime_error("Error: Invalid value for key: [background_light_color:]");
    }
    background_light_color = color;
  }

  // Carga configuración desde archivo
  void load_config(std::string const & path, config & out) {
    using Handler = void (*)(std::vector<std::string> const &, config &);
    std::unordered_map<std::string, Handler> const handlers = {
      {          "aspect_ratio",           handle_aspect_ratio},
      {           "image_width",            handle_image_width},
      {                 "gamma",                  handle_gamma},
      {       "camera_position",        handle_camera_position},
      {         "camera_target",          handle_camera_target},
      {          "camera_north",           handle_camera_north},
      {         "field_of_view",          handle_field_of_view},
      {     "samples_per_pixel",      handle_samples_per_pixel},
      {             "max_depth",              handle_max_depth},
      {     "material_rng_seed",      handle_material_rng_seed},
      {          "ray_rng_seed",           handle_ray_rng_seed},
      { "background_dark_color",  handle_background_dark_color},
      {"background_light_color", handle_background_light_color},
    };

    std::ifstream ifs(path);
    if (!ifs) {
      throw std::runtime_error("Error: Cannot open config file: " + path);
    }

    process_lines(ifs, handlers, out);
  }

}  // namespace render
