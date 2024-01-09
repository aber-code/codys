include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(codys_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(codys_setup_options)
  option(codys_ENABLE_HARDENING "Enable hardening" ON)
  option(codys_ENABLE_COVERAGE "Enable coverage reporting" ON)
  cmake_dependent_option(
    codys_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    codys_ENABLE_HARDENING
    OFF)

  codys_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR codys_PACKAGING_MAINTAINER_MODE)
    option(codys_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(codys_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(codys_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(codys_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(codys_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(codys_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(codys_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(codys_ENABLE_PCH "Enable precompiled headers" OFF)
    option(codys_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(codys_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(codys_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(codys_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(codys_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(codys_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(codys_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(codys_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(codys_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(codys_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(codys_ENABLE_PCH "Enable precompiled headers" OFF)
    option(codys_ENABLE_CACHE "Enable ccache" OFF)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      codys_ENABLE_IPO
      codys_WARNINGS_AS_ERRORS
      codys_ENABLE_USER_LINKER
      codys_ENABLE_SANITIZER_ADDRESS
      codys_ENABLE_SANITIZER_LEAK
      codys_ENABLE_SANITIZER_UNDEFINED
      codys_ENABLE_SANITIZER_THREAD
      codys_ENABLE_SANITIZER_MEMORY
      codys_ENABLE_UNITY_BUILD
      codys_ENABLE_CLANG_TIDY
      codys_ENABLE_CPPCHECK
      codys_ENABLE_COVERAGE
      codys_ENABLE_PCH
      codys_ENABLE_CACHE)
  endif()

  codys_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (codys_ENABLE_SANITIZER_ADDRESS OR codys_ENABLE_SANITIZER_THREAD OR codys_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(codys_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(codys_global_options)
  if(codys_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    codys_enable_ipo()
  endif()

  codys_supports_sanitizers()

  if(codys_ENABLE_HARDENING AND codys_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR codys_ENABLE_SANITIZER_UNDEFINED
       OR codys_ENABLE_SANITIZER_ADDRESS
       OR codys_ENABLE_SANITIZER_THREAD
       OR codys_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${codys_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${codys_ENABLE_SANITIZER_UNDEFINED}")
    codys_enable_hardening(codys_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(codys_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(codys_warnings INTERFACE)
  add_library(codys_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  codys_set_project_warnings(
    codys_warnings
    ${codys_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(codys_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(codys_options)
  endif()

  include(cmake/Sanitizers.cmake)
  codys_enable_sanitizers(
    codys_options
    ${codys_ENABLE_SANITIZER_ADDRESS}
    ${codys_ENABLE_SANITIZER_LEAK}
    ${codys_ENABLE_SANITIZER_UNDEFINED}
    ${codys_ENABLE_SANITIZER_THREAD}
    ${codys_ENABLE_SANITIZER_MEMORY})

  set_target_properties(codys_options PROPERTIES UNITY_BUILD ${codys_ENABLE_UNITY_BUILD})

  if(codys_ENABLE_PCH)
    target_precompile_headers(
      codys_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(codys_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    codys_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(codys_ENABLE_CLANG_TIDY)
    codys_enable_clang_tidy(codys_options ${codys_WARNINGS_AS_ERRORS})
  endif()

  if(codys_ENABLE_CPPCHECK)
    codys_enable_cppcheck(${codys_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(codys_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    codys_enable_coverage(codys_options)
  endif()

  if(codys_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(codys_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(codys_ENABLE_HARDENING AND NOT codys_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR codys_ENABLE_SANITIZER_UNDEFINED
       OR codys_ENABLE_SANITIZER_ADDRESS
       OR codys_ENABLE_SANITIZER_THREAD
       OR codys_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    codys_enable_hardening(codys_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
