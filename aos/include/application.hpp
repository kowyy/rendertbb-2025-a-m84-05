#ifndef AOS_APPLICATION_HPP
#define AOS_APPLICATION_HPP

#include <gsl/gsl>

namespace render {

class Application {
public:
    static int run(gsl::span<const char* const> args);
};

} // namespace render

#endif // AOS_APPLICATION_HPP
