find_package(fmt)
find_package(spdlog)
find_package(docopt)
find_package(mp-units)
Set(Boost_INSTALL_DIR ${CONAN_BOOST_ROOT})
find_package(Boost 1.73.0 COMPONENTS program_options REQUIRED)

# Generic test that uses conan libs
add_executable(codys_example main.cpp)
target_link_libraries(
  codys_example
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
          spdlog::spdlog
          mp-units::mp-units
          Boost::program_options)
