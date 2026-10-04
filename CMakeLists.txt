# Copyright (c) 2020, tevador <tevador@gmail.com>

cmake_minimum_required(VERSION 3.13)

option(WOWNERO_SEED_DEMO "Build a demo executable for wownero-seed")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif()

project(wownero-seed)

include(GNUInstallDirs)

set(WOWNERO_SEED_SOURCES
  src/argon2/blake2/blake2b.c
  src/argon2/argon2.c
  src/argon2/core.c
  src/argon2/ref.c
  src/galois_field.cpp
  src/gf_elem.cpp
  src/gf_poly.cpp
  src/heights.cpp
  src/wownero_seed.cpp
  src/pbkdf2.c
  src/reed_solomon_code.cpp
  src/secure_random.cpp
  src/wordlist.cpp)

# ld64-711 (contrib/depends/_native/bin/ld) SIGSEGVs in its order-pass
# std::sort when these objects are separate inputs to the x86_64 dylib.
# The comparer is not a strict weak ordering for that set, so libc++ walks
# off the atom array. Monero never links this library. Folding the objects
# into one relocatable file gives them a single file ordinal and the same
# local linker finishes. -keep_private_externs keeps the symbols visible
# to the final link; without it, ld -r turns them into statics.
if(HOST_ABI STREQUAL "x86_64-apple-darwin")
  add_library(wownero-seed-objects OBJECT ${WOWNERO_SEED_SOURCES})
  set_property(TARGET wownero-seed-objects PROPERTY CXX_STANDARD 11)
  set_property(TARGET wownero-seed-objects PROPERTY POSITION_INDEPENDENT_CODE ON)
  target_include_directories(wownero-seed-objects PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/wownero_seed>)

  set(WOWNERO_SEED_COMBINED "${CMAKE_CURRENT_BINARY_DIR}/wownero-seed-combined.o")
  separate_arguments(WOWNERO_SEED_CXX_FLAGS NATIVE_COMMAND "${CMAKE_CXX_FLAGS}")
  add_custom_command(
    OUTPUT "${WOWNERO_SEED_COMBINED}"
    COMMAND ${CMAKE_CXX_COMPILER}
      ${WOWNERO_SEED_CXX_FLAGS}
      -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
      -isysroot ${CMAKE_OSX_SYSROOT}
      --sysroot=${CMAKE_OSX_SYSROOT}
      -r -nostdlib -Wl,-keep_private_externs
      -o "${WOWNERO_SEED_COMBINED}"
      "$<TARGET_OBJECTS:wownero-seed-objects>"
    DEPENDS wownero-seed-objects
    COMMAND_EXPAND_LISTS
    VERBATIM
    COMMENT "Merging wownero-seed objects so ld64 can order __text"
  )
  add_library(${PROJECT_NAME} STATIC "${WOWNERO_SEED_COMBINED}")
  set_source_files_properties("${WOWNERO_SEED_COMBINED}" PROPERTIES
    EXTERNAL_OBJECT TRUE
    GENERATED TRUE)
  set_property(TARGET ${PROJECT_NAME} PROPERTY LINKER_LANGUAGE CXX)
else()
  add_library(${PROJECT_NAME} ${WOWNERO_SEED_SOURCES})
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/wownero_seed>)

if(WOWNERO_SEED_DEMO)
  add_executable(demo src/main.cpp)
  set_property(TARGET demo PROPERTY CXX_STANDARD 11)
  target_link_libraries(demo ${PROJECT_NAME})
endif()

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME}
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
