#ifndef RENDER_SCENE_PARSER_HPP
#define RENDER_SCENE_PARSER_HPP

#include <string>

namespace render {

  class scene;

}

namespace render {

  // Carga una escena desde un archivo de texto con materiales y objetos
  void parse_scene_file(std::string const & path, scene & scn);

}  // namespace render

#endif
