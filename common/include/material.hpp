#ifndef RENDER_MATERIAL_HPP
#define RENDER_MATERIAL_HPP

#include "vector.hpp"
#include <random>
#include <string>

namespace render {

  class ray;
  struct hit_record;

  // Resultado de la interacción de un rayo con una superficie
  struct scatter_result {
    bool scattered{false};        // Indica si el rayo fue dispersado
    vector attenuation{0, 0, 0};  // Factor de atenuación del color
  };

  // Clase base abstracta para todos los materiales
  class material {
  public:
    virtual ~material() = default;

    material(material const &)             = delete;
    material & operator=(material const &) = delete;
    material(material &&)                  = default;
    material & operator=(material &&)      = default;

    // Devuelve el color de reflectancia del material
    [[nodiscard]] virtual vector get_reflectance() const = 0;

    // Devuelve el tipo de material
    [[nodiscard]] virtual std::string get_type() const = 0;

    // Calcula el rayo dispersado tras una intersección
    [[nodiscard]] virtual scatter_result scatter(ray const & r_in, hit_record const & rec,
                                                 ray & scattered, std::mt19937_64 & rng) const = 0;

  protected:
    material() = default;
  };

  // Material matte
  class matte_material : public material {
  public:
    explicit matte_material(vector const & reflectance_color);

    [[nodiscard]] vector get_reflectance() const override;
    [[nodiscard]] std::string get_type() const override;
    [[nodiscard]] scatter_result scatter(ray const & r_in, hit_record const & rec, ray & scattered,
                                         std::mt19937_64 & rng) const override;

  private:
    vector reflectance;
  };

  // Material metálico
  class metal_material : public material {
  public:
    metal_material(vector const & reflectance_color, double diffusion_factor);

    [[nodiscard]] vector get_reflectance() const override;
    [[nodiscard]] double get_diffusion() const;
    [[nodiscard]] std::string get_type() const override;
    [[nodiscard]] scatter_result scatter(ray const & r_in, hit_record const & rec, ray & scattered,
                                         std::mt19937_64 & rng) const override;

  private:
    vector reflectance;
    double diffusion;
  };

  // Material transparente con refracción
  class refractive_material : public material {
  public:
    explicit refractive_material(double refraction_index);

    [[nodiscard]] vector get_reflectance() const override;
    [[nodiscard]] double get_refraction_index() const;
    [[nodiscard]] std::string get_type() const override;
    [[nodiscard]] scatter_result scatter(ray const & r_in, hit_record const & rec, ray & scattered,
                                         std::mt19937_64 & rng) const override;

  private:
    double refraction_idx;  // Índice de refracción del material
  };

}  // namespace render

#endif
