add_executable(render-par)
target_sources(render-par
    PRIVATE
      src/main.cpp
      src/application.cpp
)
target_include_directories(render-par PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(render-par PRIVATE Microsoft.GSL::GSL common TBB::tbb)

target_link_libraries(render-par PRIVATE 
    Microsoft.GSL::GSL 
    common 
    TBB::tbb
)
