#ifndef RENDER_OBJECT_HPP
#define RENDER_OBJECT_HPP

#include "material.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <string>

namespace render {

  // Información sobre la intersección de un rayo con un objeto
  struct hit_record {
    vector point{0.0, 0.0, 0.0};        // Punto de intersección
    vector normal{0.0, 0.0, 0.0};       // Vector normal en el punto
    material const * mat_ptr{nullptr};  // Material del objeto
    double t{0.0};                      // Distancia desde origen del rayo
    bool front_face{false};             // true si el rayo golpea desde fuera
  };

  // Clase base abstracta para objetos 3D
  class object {
  public:
    virtual ~object()                  = default;
    object(object const &)             = delete;
    object & operator=(object const &) = delete;
    object(object &&)                  = default;
    object & operator=(object &&)      = default;

    // Determina si el rayo interseca el objeto en el rango [t_min, t_max]
    [[nodiscard]] virtual bool hit(ray const & r, double t_min, double t_max,
                                   hit_record & rec) const = 0;

    [[nodiscard]] material const * get_material() const;
    [[nodiscard]] virtual std::string get_type() const = 0;
    [[nodiscard]] virtual vector get_center() const    = 0;
    [[nodiscard]] virtual double get_radius() const    = 0;

  protected:
    explicit object(material const * mat);

  private:
    material const * material_ptr;
  };

  // Esfera definida por centro y radio
  class sphere : public object {
  public:
    sphere(vector const & sphere_center, double sphere_radius, material const * mat);

    [[nodiscard]] bool hit(ray const & r, double t_min, double t_max,
                           hit_record & rec) const override;
    [[nodiscard]] std::string get_type() const override;
    [[nodiscard]] vector get_center() const override;
    [[nodiscard]] double get_radius() const override;

  private:
    vector center;
    double radius;
    double inv_radius;
  };

  // Cilindro definido por centro, radio y vector eje
  class cylinder : public object {
  public:
    cylinder(vector const & cylinder_center, double cylinder_radius, vector const & axis_vector,
             material const * mat);

    [[nodiscard]] bool hit(ray const & r, double t_min, double t_max,
                           hit_record & rec) const override;
    [[nodiscard]] std::string get_type() const override;
    [[nodiscard]] vector get_center() const override;
    [[nodiscard]] double get_radius() const override;
    [[nodiscard]] vector get_axis() const;
    [[nodiscard]] double get_height() const;

  private:
    vector center;
    double radius;
    vector axis;
    vector axis_normalized;
    double height;

    // Parámetros para cálculo de intersección con tapas
    struct cap_params {
      vector center;
      vector normal;
    };

    struct t_range {
      double min;
      double max;
    };

    // Métodos auxiliares para cálculo de intersecciones
    [[nodiscard]] bool hit_curved_surface(ray const & r, double t_min, double t_max,
                                          hit_record & rec) const;
    [[nodiscard]] bool hit_cap(ray const & r, cap_params const & cap, t_range range,
                               hit_record & rec) const;
  };

}  // namespace render

#endif
