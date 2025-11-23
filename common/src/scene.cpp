#include "scene.hpp"
#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"
#include <memory>
#include <string>
#include <utility>

namespace render {

  void scene::add_material(std::string const & name, std::unique_ptr<material> mat) {
    materials[name] = std::move(mat);
  }

  void scene::add_object(std::unique_ptr<object> obj) {
    objects.push_back(std::move(obj));
  }

  material const * scene::get_material(std::string const & name) const {
    auto const it = materials.find(name);
    if (it == materials.end()) {
      return nullptr;
    }
    return it->second.get();
  }

  // Encuentra la intersección más cercana entre el rayo y cualquier objeto
  bool scene::hit(ray const & r, double t_min, double t_max, hit_record & rec) const {
    hit_record temp_rec;
    bool hit_anything   = false;
    auto closest_so_far = t_max;

    // Iterar sobre todos los objetos buscando la intersección más cercana
    for (auto const & object_ptr : objects) {
      if (object_ptr->hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything   = true;
        closest_so_far = temp_rec.t;
        rec            = temp_rec;
      }
    }

    return hit_anything;
  }

}  // namespace render
