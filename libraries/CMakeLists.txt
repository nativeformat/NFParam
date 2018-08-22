# Setup unit test framework
if (NOT TARGET Catch2)
  set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Catch2/single_include)
  add_library(Catch2 INTERFACE)
  target_include_directories(Catch2 INTERFACE ${CATCH_INCLUDE_DIR})
endif()
