cmake_minimum_required(VERSION 3.16)
project(machine_learning)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(raylib)

set(SHAPE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/shape.c
  )

add_executable(shape)
target_sources(shape
  PRIVATE
  ${SHAPE_SRC}
  )

target_link_libraries(shape
  ${raylib_LIBRARIES}
  m
  )
