cmake_minimum_required(VERSION 3.5)

project(NFParam)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
  -Wno-deprecated-declarations \
  -Wno-tautological-undefined-compare \
  -stdlib=libc++")

if(USE_ADDRESS_SANITIZER)
  message("Using Address & Leak Sanitizer")
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
  set(
    CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
endif()

if(CODE_COVERAGE)
  message("Using Code Coverage")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

set(NFPARAM_INCLUDE_DIRECTORY
  "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE STRING "NFParam Includes" FORCE)

add_subdirectory(libraries)
add_subdirectory(source)
