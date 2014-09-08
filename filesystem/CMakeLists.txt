# $Id$
project(FILESYSTEM)

########### Qt4 configuration #########
if(USE_QT5)

  find_package(Qt5Widgets REQUIRED)

else()

  find_package(Qt4 REQUIRED)
  set(QT_USE_QTXML TRUE)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})

endif()

########### includes ###############
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/base-qt)

########### next target ###############
set(filesystem_SOURCES
  FileIconProvider.cpp
  FileInformationDialog.cpp
  FileRecordToolTipWidget.cpp
  FileSystemFrame.cpp
  FileSystemModel.cpp
  RemoveFilesDialog.cpp
  RenameFileDialog.cpp
)

add_library(filesystem STATIC ${filesystem_SOURCES})
target_link_libraries(filesystem ${QT_LIBRARIES})

if(USE_QT5)

  qt5_use_modules(filesystem Widgets Network Xml)

endif()
