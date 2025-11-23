#include "object.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>

namespace render {
  namespace {

    // Distancia mínima para considerar intersecciones válidas
    constexpr double min_hit_distance = 1e-3;

    inline bool is_in_range(double value, double min, double max) {
      return value >= min and value <= max;
    }

    // Coeficientes de ecuación cuadrática
    struct Quad {
      double a, b, c;
    };

    // Calcula la ecuación cuadrática para intersección con superficie curva del cilindro
    inline Quad cylinder_quad(render::vector const & rc, render::vector const & dr,
                              render::vector const & axis_n, double radius) {
      auto const rc_perp = rc.perpendicular_to(axis_n);
      auto const dr_perp = dr.perpendicular_to(axis_n);
      double const a     = render::vector::dot(dr_perp, dr_perp);
      double const b     = 2.0 * render::vector::dot(rc_perp, dr_perp);
      double const c     = render::vector::dot(rc_perp, rc_perp) - radius * radius;
      return {a, b, c};
    }

    struct Range {
      double min, max;
    };

    // Resuelve la ecuación cuadrática y devuelve la raíz válida más cercana
    inline std::optional<double> choose_root(Quad const & q, Range range) {
      double const disc = q.b * q.b - 4.0 * q.a * q.c;
      if (disc < 0.0) {
        return std::nullopt;
      }

      double const sqrt_disc = std::sqrt(disc);
      double const two_a     = 2.0 * q.a;
      auto const eff_min     = std::max(range.min, min_hit_distance);

      // Probar raíz menor primero
      double t = (-q.b - sqrt_disc) / two_a;
      if (t >= eff_min and t <= range.max) {
        return t;
      }

      // Probar raíz mayor
      t = (-q.b + sqrt_disc) / two_a;
      if (t >= eff_min and t <= range.max) {
        return t;
      }

      return std::nullopt;
    }

    // Verifica si un punto está dentro de las tapas del cilindro
    inline bool within_caps(render::vector const & p, render::vector const & center,
                            render::vector const & axis_n, double height) {
      constexpr double cap_epsilon = 1e-8;
      double const axial_distance  = std::abs(render::vector::dot(p - center, axis_n));
      return axial_distance <= (height * 0.5 + cap_epsilon);
    }

    // Calcula vector normal saliente en la superficie curva del cilindro
    inline std::optional<render::vector> outward_normal_at(render::vector const & p,
                                                           render::vector const & center,
                                                           render::vector const & axis_n) {
      auto const radial_vec   = p - center;
      double const axial_comp = render::vector::dot(radial_vec, axis_n);
      auto const radial_proj  = radial_vec - (axial_comp * axis_n);

      // Verificar que el vector no sea cero
      constexpr double eps = 1e-8;
      if (radial_proj.magnitude_squared() < eps * eps) {
        return std::nullopt;
      }

      return radial_proj;
    }

  }  // namespace

  // CLASE BASE object

  object::object(material const * mat) : material_ptr{mat} {
    if (mat == nullptr) {
      throw std::invalid_argument("Material pointer cannot be null");
    }
  }

  material const * object::get_material() const {
    return material_ptr;
  }

  // ESFERA

  sphere::sphere(vector const & sphere_center, double const sphere_radius, material const * mat)
      : object{mat}, center{sphere_center}, radius{sphere_radius}, inv_radius{1.0 / sphere_radius} {
    if (sphere_radius <= 0.0) {
      throw std::invalid_argument("Sphere radius must be positive");
    }
  }

  std::string sphere::get_type() const {
    return "sphere";
  }

  vector sphere::get_center() const {
    return center;
  }

  double sphere::get_radius() const {
    return radius;
  }

  // Intersección rayo-esfera usando ecuación cuadrática
  bool sphere::hit(ray const & r, double const t_min, double const t_max, hit_record & rec) const {
    vector const rc           = center - r.get_origin();
    vector const dr           = r.get_direction();
    double const a            = vector::dot(dr, dr);
    double const b            = -2.0 * vector::dot(dr, rc);
    double const c            = vector::dot(rc, rc) - radius * radius;
    double const discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) {
      return false;
    }

    double const sqrt_disc = std::sqrt(discriminant);
    double const two_a     = 2.0 * a;
    double t               = (-b - sqrt_disc) / two_a;

    // Buscar raíz válida más cercana
    double const effective_t_min = std::max(t_min, min_hit_distance);
    if (not is_in_range(t, effective_t_min, t_max)) {
      t = (-b + sqrt_disc) / two_a;
      if (not is_in_range(t, effective_t_min, t_max)) {
        return false;
      }
    }

    // Registrar hit
    rec.t                       = t;
    rec.point                   = r.at(t);
    rec.mat_ptr                 = get_material();
    vector const outward_normal = (rec.point - center) * inv_radius;
    rec.front_face              = vector::dot(dr, outward_normal) < 0.0;
    rec.normal                  = rec.front_face ? outward_normal : -outward_normal;

    return true;
  }

  // CILINDRO

  cylinder::cylinder(vector const & cylinder_center, double const cylinder_radius,
                     vector const & axis_vector, material const * mat)
      : object{mat}, center{cylinder_center}, radius{cylinder_radius}, axis{axis_vector},
        axis_normalized{axis_vector.normalized()}, height{axis_vector.magnitude()} {
    if (cylinder_radius <= 0.0) {
      throw std::invalid_argument("Cylinder radius must be positive");
    }
    if (axis_vector.is_near_zero()) {
      throw std::invalid_argument("Cylinder axis cannot be zero vector");
    }
  }

  std::string cylinder::get_type() const {
    return "cylinder";
  }

  vector cylinder::get_center() const {
    return center;
  }

  double cylinder::get_radius() const {
    return radius;
  }

  vector cylinder::get_axis() const {
    return axis;
  }

  double cylinder::get_height() const {
    return height;
  }

  // Intersección rayo-cilindro (superficie curva + dos tapas)
  bool cylinder::hit(ray const & r, double const t_min, double const t_max,
                     hit_record & rec) const {
    bool hit_anything = false;
    double closest    = t_max;

    // 1) Superficie curva
    if (hit_curved_surface(r, t_min, closest, rec)) {
      hit_anything = true;
      closest      = rec.t;
    }

    // 2) Tapa superior
    vector const top = center + axis_normalized * (height / 2.0);
    hit_record temp_rec;
    if (hit_cap(r, {top, axis_normalized}, {t_min, closest}, temp_rec)) {
      hit_anything = true;
      rec          = temp_rec;
      closest      = temp_rec.t;
    }

    // 3) Tapa inferior
    vector const bottom = center - axis_normalized * (height / 2.0);
    if (hit_cap(r, {bottom, -axis_normalized}, {t_min, closest}, temp_rec)) {
      hit_anything = true;
      rec          = temp_rec;
    }

    return hit_anything;
  }

  // Intersección con tapa circular del cilindro
  bool cylinder::hit_cap(ray const & r, cap_params const & cap, t_range const range,
                         hit_record & rec) const {
    vector const dr    = r.get_direction();
    double const denom = vector::dot(dr, cap.normal);

    // Verificar si el rayo es paralelo al plano
    constexpr double eps_parallel = 1e-8;
    if (std::abs(denom) < eps_parallel) {
      return false;
    }

    // Calcular distancia a intersección con plano
    double const t             = vector::dot(cap.center - r.get_origin(), cap.normal) / denom;
    double const effective_min = std::max(range.min, min_hit_distance);
    if (t < effective_min or t > range.max) {
      return false;
    }

    // Verificar si el punto está dentro del círculo
    vector const point      = r.at(t);
    vector const vcp        = point - cap.center;
    double const axial_comp = vector::dot(vcp, cap.normal);
    vector const radial_vec = vcp - axial_comp * cap.normal;
    double const rdist_sq   = radial_vec.magnitude_squared();
    double const radius_sq  = radius * radius;
    if (rdist_sq > radius_sq) {
      return false;
    }

    // Registrar hit
    rec.t       = t;
    rec.point   = point;
    rec.mat_ptr = get_material();

    vector const outward_normal = cap.normal;
    rec.front_face              = vector::dot(dr, outward_normal) < 0.0;
    rec.normal                  = rec.front_face ? outward_normal : -outward_normal;

    return true;
  }

  // Intersección con superficie curva del cilindro
  bool cylinder::hit_curved_surface(ray const & r, double const t_min, double const t_max,
                                    hit_record & rec) const {
    auto const rc    = r.get_origin() - center;
    auto const dr    = r.get_direction();
    auto const q     = cylinder_quad(rc, dr, axis_normalized, radius);
    auto const t_opt = choose_root(q, {t_min, t_max});

    if (!t_opt) {
      return false;
    }

    double const t   = *t_opt;
    auto const point = r.at(t);

    // Verificar que el punto esté dentro de las tapas
    if (!within_caps(point, center, axis_normalized, height)) {
      return false;
    }

    auto const n_opt = outward_normal_at(point, center, axis_normalized);
    if (!n_opt) {
      return false;
    }

    // Registrar hit
    rec.t       = t;
    rec.point   = point;
    rec.mat_ptr = get_material();

    auto const n   = *n_opt;
    rec.front_face = render::vector::dot(dr, n) < 0.0;
    rec.normal     = rec.front_face ? n : -n;

    return true;
  }

}  // namespace render
