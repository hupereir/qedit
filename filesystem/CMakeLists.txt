# $Id$
PROJECT( FILESYSTEM )

########### Qt4 configuration #########
FIND_PACKAGE( Qt4 REQUIRED )
SET( QT_USE_QTXML TRUE )

INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} )

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
  FileRecordToolTipWidget.h
  FileSystemFrame.h
  RenameFileDialog.h
)

QT4_WRAP_CPP( filesystem_HEADERS_MOC ${filesystem_HEADERS} )

ADD_LIBRARY( filesystem STATIC
  ${filesystem_SOURCES}
  ${filesystem_HEADERS_MOC}
)

TARGET_LINK_LIBRARIES( filesystem ${QT_LIBRARIES} )
