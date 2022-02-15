Set(Boost_INSTALL_DIR ${CONAN_BOOST_ROOT})
find_package(Boost 1.73.0 COMPONENTS program_options REQUIRED)

# Generic test that uses conan libs
add_library(codys INTERFACE)

target_include_directories(codys INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(codys INTERFACE
  Boost::program_options)

