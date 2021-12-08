cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(clt13 VERSION 0.1.0 LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

set(clt13_SOURCES
  src/clt.c
  src/clt_elem.c
  src/crt_tree.c
  src/estimates.c
  src/utils.c
  )

add_library(clt13 SHARED ${clt13_SOURCES})
target_link_libraries(clt13 PUBLIC gmp m aesrand)

include(FindOpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror -fPIC -std=gnu11 -march=native ${OpenMP_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pg -ggdb -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
target_link_libraries(clt13 INTERFACE "${OpenMP_C_FLAGS}")

install(TARGETS clt13 LIBRARY DESTINATION lib)
install(FILES src/clt13.h src/clt_pl.h DESTINATION include)

# Test files

enable_testing()
macro(add_test_ _name)
  add_executable("${_name}" "test/${_name}.c")
  target_include_directories("${_name}" PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries("${_name}" PRIVATE clt13 aesrand)
  add_test(NAME "${_name}" COMMAND "${_name}")
endmacro()

add_test_(test_clt)
add_test_(test_clt_state_new)
add_test_(test_clt_witness_scheme)
add_test_(compute_clt_witness)
