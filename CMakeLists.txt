
add_subdirectory(meta)

file(GLOB  SOURCES *.cpp)

add_library(micro ${SOURCES})
target_include_directories(micro PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
