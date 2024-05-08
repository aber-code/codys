
# Generic test that uses conan libs
add_library(codys INTERFACE)

target_include_directories(codys INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(codys INTERFACE
  mp-units::mp-units
  fmt::fmt )

