#include "application.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <gsl/gsl>

int main(int argc, char ** argv) {
    try {
        return render::Application::run({argv, static_cast<size_t>(argc)});
    } catch (std::exception const & e) {
        std::cerr << "Ha ocurrido una excepción: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Ha ocurrido un error desconocido.\n";
        return EXIT_FAILURE;
    }
}
