#ifndef RENDER_SCENE_HPP
#define RENDER_SCENE_HPP

#include "object.hpp"
#include "ray.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace render {

  // Contenedor de objetos y materiales de la escena 3D
  class scene {
  public:
    scene() = default;

    // Añade un material con nombre único a la escena
    void add_material(std::string const & name, std::unique_ptr<material> mat);

    // Añade un objeto geométrico a la escena
    void add_object(std::unique_ptr<object> obj);

    // Determina si un rayo interseca algún objeto en el rango [t_min, t_max]
    [[nodiscard]] bool hit(ray const & r, double t_min, double t_max, hit_record & rec) const;

    // Obtiene material por nombre
    [[nodiscard]] material const * get_material(std::string const & name) const;

  private:
    std::map<std::string, std::unique_ptr<material>> materials;
    std::vector<std::unique_ptr<object>> objects;
  };

}  // namespace render

#endif
