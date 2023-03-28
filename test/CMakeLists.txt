find_package(Catch2 REQUIRED)
find_package(mp-units)

include(CTest)
include(Catch)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_link_libraries(catch_main PRIVATE myproject::project_options)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE 
  mp-units::mp-units 
  codys
  myproject::project_warnings 
  myproject::project_options
  catch_main)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  tests
  TEST_PREFIX
  "unittests."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)
