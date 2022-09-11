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




