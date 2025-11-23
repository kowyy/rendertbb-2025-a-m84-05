set(COMMON_SRC_FILES
  "${CMAKE_SOURCE_DIR}/soa/src/application.cpp"
)

set(CURRENT_DIR_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/test_render_soa.cpp"
)

add_unit_test_target(
  TARGET_NAME utest-soa
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER soa
  COVERAGE_DIR coverage-soa
  LIBRARY_TO_LINK common
  INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/soa/include
)
