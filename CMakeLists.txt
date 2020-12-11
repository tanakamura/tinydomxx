cmake_minimum_required(VERSION 3.4)
project(tinydomxx)

add_executable(tinydomxx_test
  tinydomxx_test.cpp)

target_link_options(tinydomxx_test
  PRIVATE --js-library ${CMAKE_CURRENT_SOURCE_DIR}/libtinydomxx_emscripten.js -s EXPORTED_FUNCTIONS=["_malloc","_free"]
)

