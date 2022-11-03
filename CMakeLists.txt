cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
  darts
  VERSION 1.0
  LANGUAGES CXX
)

# ============================================================================
# Configure dependencies
# ============================================================================
include(cmake/DartsConfig.cmake)

# ============================================================================
# Create darts library
# ============================================================================

set(darts_lib_SOURCES)
list(
  APPEND
  darts_lib_SOURCES
  # cmake-format: off
  # Additional files for PA6 below
  include/darts/medium.h
  include/darts/photon.h
  include/darts/point_kdtree.h
  src/photon.cpp
  src/media/homogeneous.cpp
  src/media/vacuum.cpp
  # Additional files for PA5 below
  src/tests/surface_sample_test.cpp
  # Additional files for PA4 below
  src/samplers/independent.cpp
  include/darts/sampler.h
  src/tests/material_sample_test.cpp
  # Additional files for PA3 below
  # Additional files for PA2 below
  include/darts/box.h
  include/darts/mesh.h
  include/darts/triangle.h
  src/surfaces/bbh.cpp
  src/surfaces/mesh.cpp
  src/surfaces/triangle.cpp
  src/tests/intersection_test.cpp
  # Additional files for PA1 below
  include/darts/camera.h
  include/darts/factory.h
  include/darts/json.h
  include/darts/material.h
  include/darts/sampling.h
  include/darts/scene.h
  include/darts/sphere.h
  include/darts/stats.h
  include/darts/surface.h
  include/darts/surface_group.h
  include/darts/test.h
  include/darts/transform.h
  src/camera.cpp
  src/example_scenes.cpp
  src/parser.cpp
  src/scene.cpp
  src/stats.cpp
  src/materials/dielectric.cpp
  src/materials/diffuse_light.cpp
  src/materials/lambertian.cpp
  src/materials/material.cpp
  src/materials/metal.cpp
  src/surfaces/quad.cpp
  src/surfaces/sphere.cpp
  src/surfaces/surface.cpp
  src/surfaces/surface_group.cpp
  src/tests/material_scatter_test.cpp
  src/tests/test.cpp
  # The files below are included in the PA0 basecode
  include/darts/array2d.h
  include/darts/common.h
  include/darts/fwd.h
  include/darts/image.h
  include/darts/math.h
  include/darts/parallel.h
  include/darts/progress.h
  include/darts/ray.h
  include/darts/spherical.h
  src/common.cpp
  src/image.cpp
  src/math.cpp
  src/progress.cpp
  # cmake-format: on
)


# Additional files for A6 below
if(USE_NANOVDB)
  list(APPEND darts_lib_SOURCES src/media/nanovdb_medium.cpp)
endif(USE_NANOVDB)

add_library(darts_lib OBJECT ${darts_lib_SOURCES})

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(darts_lib PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
target_link_libraries(
  darts_lib
  PRIVATE ${DARTS_PRIVATE_LIBS}
  PUBLIC ${DARTS_PUBLIC_LIBS}
)

# Windows' math include does not define constants by default. Set this definition so it does. Also
# set NOMINMAX so the min and max functions are not overwritten with macros.
if(MSVC)
  target_compile_definitions(darts_lib PUBLIC -D_USE_MATH_DEFINES -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

target_include_directories(
  darts_lib PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                   $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ============================================================================
# Add executables and link with darts_lib
# ============================================================================

add_executable(darts_tutorial0 src/darts_tutorial0.cpp)
target_link_libraries(darts_tutorial0 PRIVATE darts_lib)

add_executable(darts_tutorial1 src/darts_tutorial1.cpp)
target_link_libraries(darts_tutorial1 PRIVATE darts_lib)

add_executable(darts src/darts.cpp)
target_link_libraries(darts PRIVATE darts_lib)

add_executable(img_compare src/img_compare.cpp)
target_link_libraries(img_compare PRIVATE darts_lib)

add_executable(img_avg src/img_avg.cpp)
target_link_libraries(img_avg PRIVATE darts_lib)


if(USE_NANOVDB)
  add_executable(nanovdb_test src/nanovdb_test.cpp)
  target_link_libraries(nanovdb_test PRIVATE darts_lib nanovdb)
endif(USE_NANOVDB)

