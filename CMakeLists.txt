PROJECT( QEDIT )
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

set(QEDIT_VERSION_MAJOR 2)
set(QEDIT_VERSION_MINOR 4)
SET( QEDIT_VERSION_PATCH 6 )
set(QEDIT_VERSION ${QEDIT_VERSION_MAJOR}.${QEDIT_VERSION_MINOR}.${QEDIT_VERSION_PATCH} )

########### packages ###############
SET( CPACK_PACKAGE_NAME "qedit" )
SET( CPACK_PACKAGE_VENDOR "hugo.pereira@free.fr" )
SET( CPACK_PACKAGE_VERSION "${QEDIT_VERSION}" )
SET( CPACK_SOURCE_GENERATOR TGZ )
SET( CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}" )
INCLUDE( CPack )

########### configuration files ###############
CONFIGURE_FILE( qedit.spec.cmake ${CMAKE_SOURCE_DIR}/qedit.spec )

########### external applications ###############
FIND_PROGRAM( ASPELL aspell )
FIND_PROGRAM( DIFF diff )

########### aspell ###############
FIND_PACKAGE( ASPELL )

########### links #########
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LINK_DIRECTORIES( ${CMAKE_INSTALL_PREFIX}/lib )

########### subdirectories ###############
IF( NOT ENABLE_SHARED )

  ADD_SUBDIRECTORY( base )
  ADD_SUBDIRECTORY( base-qt )
  ADD_SUBDIRECTORY( base-server )
  ADD_SUBDIRECTORY( base-help )

  IF( ASPELL_FOUND )
    ADD_SUBDIRECTORY( base-spellcheck )
  ENDIF( ASPELL_FOUND )

ENDIF( NOT ENABLE_SHARED )

ADD_SUBDIRECTORY( document-classes )
ADD_SUBDIRECTORY( filesystem )
ADD_SUBDIRECTORY( src )
