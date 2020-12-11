cmake_minimum_required(VERSION 3.4)
project(tinydomxx)

add_executable(tinydomxx_test
  tinydomxx_test.cpp)

add_custom_target(test_html ALL 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test.html
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.html
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.html ${CMAKE_CURRENT_BINARY_DIR}/test.html
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.html)

target_link_options(tinydomxx_test
  PRIVATE --js-library ${CMAKE_CURRENT_SOURCE_DIR}/libtinydomxx_emscripten.js -s EXPORTED_FUNCTIONS=["_malloc","_free"]
)

