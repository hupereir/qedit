# $Id$
cmake_minimum_required(VERSION 2.8.12)

project(qedit)
set(QEDIT_VERSION_MAJOR 2)
SET(QEDIT_VERSION_MINOR 8)
SET(QEDIT_VERSION_PATCH 1)
set(QEDIT_VERSION ${QEDIT_VERSION_MAJOR}.${QEDIT_VERSION_MINOR}.${QEDIT_VERSION_PATCH})

########### modules #################
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/base-cmake")
include(Macros)
setup_cmake_settings()
setup_compiler_flags()

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
find_program(XDG_OPEN xdg-open)

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

  endif()

  add_subdirectory(base-filesystem)

endif()

add_subdirectory(document-classes)
add_subdirectory(src)
