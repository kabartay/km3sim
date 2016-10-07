cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(km3sim)

set(CMAKE_C_FLAGS "-Wall -g")
set(DOCOPT_ROOT ${PROJECT_SOURCE_DIR}/external/docopt)
set(DOCOPT_INCLUDE_DIRS ${DOCOPT_ROOT}/include/docopt)
set(DOCOPT_LIBRARIES ${DOCOPT_ROOT}/lib64/libdocopt.a)
set(docopt_INSTALL_DIR "${DOCOPT_ROOT}")
set(docopt_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${docopt_INSTALL_DIR})
include(ExternalProject)
ExternalProject_Add(docopt
  PREFIX ${DOCOPT_ROOT}
  GIT_REPOSITORY https://github.com/docopt/docopt.cpp.git
  BINARY_DIR ${DOCOPT_ROOT}
  INSTALL_DIR ${DOCOPT_ROOT}
  CMAKE_ARGS ${docopt_CMAKE_ARGS}
)
add_library(libdocopt STATIC IMPORTED)
set_target_properties(libdocopt PROPERTIES IMPORTED_LOCATION ${DOCOPT_LIBRARIES})
add_dependencies(libdocopt docopt)

find_package(Geant4 10 REQUIRED)
include(${Geant4_USE_FILE})
set(Geant4_INCLUDE_DIRS ${Geant4_DIR}/include/Geant4)

include_directories(${DOCOPT_INCLUDE_DIRS})
include_directories(${Geant4_INCLUDE_DIRS})
add_definitions(${Geant4_DEFINITIONS})
set(CMAKE_CXX_FLAGS ${Geant4_CXX_FLAGS})

include_directories(${PROJECT_SOURCE_DIR}/src)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.h)

add_executable(km3sim km3sim.cc ${sources} ${headers})
target_link_libraries(km3sim ${Geant4_LIBRARIES})
target_link_libraries(km3sim libdocopt)
