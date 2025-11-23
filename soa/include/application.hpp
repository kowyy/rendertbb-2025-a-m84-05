#ifndef SOA_APPLICATION_HPP
#define SOA_APPLICATION_HPP

#include <gsl/gsl>

namespace render {

// Clase de la aplicación del render
class Application {
public:
  static int run(gsl::span<const char* const> args);
};

} // namespace render

#endif // SOA_APPLICATION_HPP
