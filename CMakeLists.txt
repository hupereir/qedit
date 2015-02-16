# $Id$
cmake_minimum_required(VERSION 2.8.12)

project(qedit)
set(QEDIT_VERSION_MAJOR 2)
SET(QEDIT_VERSION_MINOR 7)
SET(QEDIT_VERSION_PATCH 0)
set(QEDIT_VERSION ${QEDIT_VERSION_MAJOR}.${QEDIT_VERSION_MINOR}.${QEDIT_VERSION_PATCH})

########### modules #################
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/base-cmake")
include(Macros)
setup_cmake_settings()
setup_compiler_flags()

########### timestamp #########
get_build_timestamp(TIMESTAMP)

########### packages ###############
set(CPACK_PACKAGE_NAME "qedit")
set(CPACK_PACKAGE_VENDOR "hugo.pereira@free.fr")
set(CPACK_PACKAGE_VERSION "${QEDIT_VERSION}")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
include(CPack)

########### configuration files ###############
configure_file(qedit.spec.cmake ${CMAKE_SOURCE_DIR}/qedit.spec)

########### external applications ###############
find_program(DIFF diff)

########### aspell ###############
find_program(ASPELL aspell)
find_package(ASPELL)
if(ASPELL_FOUND)
  add_definitions(-DUSE_ASPELL=1)
else()
  add_definitions(-DUSE_ASPELL=0)
endif()

########### subdirectories ###############
if(ENABLE_SHARED)

  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  link_directories(${CMAKE_INSTALL_PREFIX}/lib)

else()

  add_subdirectory(base)
  add_subdirectory(base-qt)
  add_subdirectory(base-server)
  add_subdirectory(base-help)

  if(ASPELL_FOUND)

    add_subdirectory(base-spellcheck)

  endif(ASPELL_FOUND)

endif()

add_subdirectory(document-classes)
add_subdirectory(filesystem)
add_subdirectory(src)
