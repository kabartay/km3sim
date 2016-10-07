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

find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/dox.cfg)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()
