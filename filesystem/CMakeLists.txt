# $Id$
PROJECT( FILESYSTEM )

########### Qt4 configuration #########
IF( USE_QT5 )

  FIND_PACKAGE( Qt5Widgets REQUIRED )

ELSE()

  FIND_PACKAGE( Qt4 REQUIRED )
  SET( QT_USE_QTXML TRUE )
  INCLUDE( ${QT_USE_FILE} )
  ADD_DEFINITIONS( ${QT_DEFINITIONS} )

ENDIF()

########### includes ###############
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-qt)

########### next target ###############
SET( filesystem_SOURCES
  FileIconProvider.cpp
  FileInformationDialog.cpp
  FileRecordToolTipWidget.cpp
  FileSystemFrame.cpp
  FileSystemModel.cpp
  RemoveFilesDialog.cpp
  RenameFileDialog.cpp
)

SET( filesystem_HEADERS
  FileInformationDialog.h
  FileRecordToolTipWidget.h
  FileSystemFrame.h
  FileSystemModel.h
  RemoveFilesDialog.h
  RenameFileDialog.h
)

IF( USE_QT5 )

  QT5_WRAP_CPP( filesystem_HEADERS_MOC ${filesystem_HEADERS} )

ELSE()

  QT4_WRAP_CPP( filesystem_HEADERS_MOC ${filesystem_HEADERS} )

ENDIF()

ADD_LIBRARY( filesystem STATIC
  ${filesystem_SOURCES}
  ${filesystem_HEADERS_MOC}
)

TARGET_LINK_LIBRARIES( filesystem ${QT_LIBRARIES} )

IF( USE_QT5 )

  QT5_USE_MODULES( filesystem Widgets Network Xml )

ENDIF()
