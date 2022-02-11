find_package(fmt)
find_package(spdlog)
find_package(docopt)
find_package(mp-units)

# Generic test that uses conan libs
add_executable(codys_example main.cpp)
target_link_libraries(
  codys_example
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
          spdlog::spdlog
          mp-units::mp-units)
