#include "material.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>

namespace {

  // Genera vector aleatorio con componentes en [-1, 1]
  render::vector random_vector_components(std::mt19937_64 & rng) {
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    return render::vector{dist(rng), dist(rng), dist(rng)};
  }

  // Genera vector de difusión para materiales metálicos
  render::vector random_diffusion_vector(std::mt19937_64 & rng, double diffusion) {
    std::uniform_real_distribution<double> dist(-diffusion, diffusion);
    return render::vector{dist(rng), dist(rng), dist(rng)};
  }

}  // namespace

namespace render {

  namespace {

    // Valida que todas las componentes de reflectancia estén en [0, 1]
    void validate_reflectance(vector const & refl) {
      if (refl.x < 0.0 or
          refl.x > 1.0 or
          refl.y < 0.0 or
          refl.y > 1.0 or
          refl.z < 0.0 or
          refl.z > 1.0)
      {
        throw std::invalid_argument("Reflectance components must be in range [0, 1]");
      }
    }

  }  // namespace

  // MATERIAL MATE
  matte_material::matte_material(vector const & reflectance_color)
      : reflectance{reflectance_color} {
    validate_reflectance(reflectance_color);
  }

  vector matte_material::get_reflectance() const {
    return reflectance;
  }

  std::string matte_material::get_type() const {
    return "matte";
  }

  scatter_result matte_material::scatter(ray const & /*r_in*/, hit_record const & rec,
                                         ray & scattered, std::mt19937_64 & rng) const {
    scatter_result result;

    // Genera dirección aleatoria alrededor de la normal
    vector scatter_direction = rec.normal + random_vector_components(rng);

    // Si la dirección resultante es casi cero, usa la normal directamente
    if (scatter_direction.is_near_zero()) {
      scatter_direction = rec.normal;
    }

    scattered          = ray{rec.point, scatter_direction};
    result.attenuation = reflectance;
    result.scattered   = true;

    return result;
  }

  // MATERIAL METÁLICO
  metal_material::metal_material(vector const & reflectance_color, double const diffusion_factor)
      : reflectance{reflectance_color}, diffusion{diffusion_factor} {
    validate_reflectance(reflectance_color);
    if (diffusion_factor < 0.0) {
      throw std::invalid_argument("Diffusion factor must be non-negative");
    }
  }

  vector metal_material::get_reflectance() const {
    return reflectance;
  }

  double metal_material::get_diffusion() const {
    return diffusion;
  }

  std::string metal_material::get_type() const {
    return "metal";
  }

  scatter_result metal_material::scatter(ray const & r_in, hit_record const & rec, ray & scattered,
                                         std::mt19937_64 & rng) const {
    scatter_result result;

    vector const direction_in = r_in.get_direction();

    // Calcula reflexión especular
    vector const reflected =
        direction_in - 2.0 * vector::dot(direction_in, rec.normal) * rec.normal;

    vector const reflected_hat = reflected.normalized();

    // Añade difusión aleatoria para simular rugosidad
    vector const fuzz_vec    = random_diffusion_vector(rng, diffusion);
    vector const scatter_dir = reflected_hat + fuzz_vec;

    scattered          = ray{rec.point, scatter_dir};
    result.attenuation = reflectance;
    result.scattered   = true;

    return result;
  }

  // MATERIAL REFRACTIVO
  refractive_material::refractive_material(double refraction_index)
      : refraction_idx{refraction_index} {
    if (std::abs(refraction_index) < epsilon or refraction_index < 0.0) {
      throw std::invalid_argument("Refraction index must be positive");
    }
  }

  vector refractive_material::get_reflectance() const {
    return vector{1.0, 1.0, 1.0};
  }

  double refractive_material::get_refraction_index() const {
    return refraction_idx;
  }

  std::string refractive_material::get_type() const {
    return "refractive";
  }

  scatter_result refractive_material::scatter(ray const & r_in, hit_record const & rec,
                                              ray & scattered, std::mt19937_64 & /*rng*/) const {
    scatter_result result;
    result.attenuation = vector{1.0, 1.0, 1.0};

    // Ratio de refracción si entra o sale del material
    double const refraction_ratio = rec.front_face ? (1.0 / refraction_idx) : refraction_idx;

    vector const unit_direction = r_in.get_direction().normalized();

    // Cálculo del ángulo de incidencia
    double const cos_theta = std::min(vector::dot(-unit_direction, rec.normal), 1.0);
    double const sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

    // Comprueba si hay reflexión total interna
    bool const cannot_refract = refraction_ratio * sin_theta > 1.0;

    vector direction;
    if (cannot_refract) {
      // Reflexión total interna
      direction = unit_direction - 2.0 * vector::dot(unit_direction, rec.normal) * rec.normal;
    } else {
      // Refracción
      vector const r_out_perp      = refraction_ratio * (unit_direction + cos_theta * rec.normal);
      double const perp_mag_sq     = r_out_perp.magnitude_squared();
      double const parallel_mag_sq = std::max(0.0, 1.0 - perp_mag_sq);
      vector const r_out_parallel  = -std::sqrt(parallel_mag_sq) * rec.normal;
      direction                    = r_out_perp + r_out_parallel;
    }

    scattered        = ray{rec.point, direction};
    result.scattered = true;
    return result;
  }

}  // namespace render
