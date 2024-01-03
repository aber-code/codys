include(CTest)

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE 
  mp-units::mp-units 
  codys
  codys_warnings 
  codys_options
  Catch2::Catch2WithMain)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  tests
  TEST_PREFIX
  "unittests."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)

add_executable(constexpr_tests constexpr_tests.cpp)
target_link_libraries(
    constexpr_tests
    PRIVATE mp-units::mp-units 
    codys
    codys_warnings 
    codys_options
    Catch2::Catch2WithMain)

catch_discover_tests(
  constexpr_tests
  TEST_PREFIX
  "constexpr."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "constexpr."
  OUTPUT_SUFFIX
  .xml)