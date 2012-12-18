# $Id$

########### X11 #########
IF( UNIX )

  FIND_PACKAGE( X11 REQUIRED )
  INCLUDE_DIRECTORIES( ${X11_X11_INCLUDE_PATH} )

ENDIF()

########### Qt4 configuration #########
FIND_PACKAGE( Qt4 REQUIRED )
SET( QT_USE_QTXML TRUE )

INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} )

########### includes #########
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-qt)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-server)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-help)

IF( ASPELL_FOUND )
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-spellcheck)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/document-classes)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/filesystem)

########### configuration files ###############
CONFIGURE_FILE( Config.h.cmake Config.h )
CONFIGURE_FILE( SystemOptions.h.cmake SystemOptions.h )

########### next target ###############
SET( qedit_SOURCES
  Application.cpp
  AskForSaveDialog.cpp
  AutoSave.cpp
  AutoSaveThread.cpp
  ConfigurationDialog.cpp
  Diff.cpp
  DocumentClassMenu.cpp
  DocumentClassToolBar.cpp
  CloseFilesDialog.cpp
  FileCheck.cpp
  FileCheckDialog.cpp
  FileModifiedDialog.cpp
  FileRemovedDialog.cpp
  FileSelectionDialog.cpp
  HtmlHelper.cpp
  MainWindow.cpp
  Menu.cpp
  NavigationFrame.cpp
  NavigationToolBar.cpp
  NewDocumentNameServer.cpp
  NewFileDialog.cpp
  PrintHelper.cpp
  ReplaceDialog.cpp
  RecentFilesFrame.cpp
  SaveAllDialog.cpp
  SessionFilesFrame.cpp
  SessionFilesModel.cpp
  TextDisplay.cpp
  TextView.cpp
  WindowServer.cpp
  main.cpp
)

SET( qedit_HEADERS
  Application.h
  AskForSaveDialog.h
  AutoSave.h
  Diff.h
  DocumentClassMenu.h
  DocumentClassToolBar.h
  FileCheck.h
  FileModifiedDialog.h
  FileRemovedDialog.h
  FileSelectionDialog.h
  HtmlHelper.h
  MainWindow.h
  Menu.h
  NavigationFrame.h
  NavigationToolBar.h
  NewFileDialog.h
  PrintHelper.h
  ProgressDialog.h
  RecentFilesFrame.h
  ReplaceDialog.h
  SessionFilesFrame.h
  SessionFilesModel.h
  TextDisplay.h
  TextView.h
  WindowServer.h
)

SET( qedit_RESOURCES pixmaps.qrc)

QT4_WRAP_CPP( qedit_HEADERS_MOC ${qedit_HEADERS} )
QT4_ADD_RESOURCES( qedit_RESOURCES_RCC ${qedit_RESOURCES})

ADD_PLATFORM_EXECUTABLE( qedit
  ${QEDIT_VERSION}
  ${qedit_SOURCES}
  ${qedit_HEADERS_MOC}
  ${qedit_RESOURCES_RCC}
)

TARGET_LINK_LIBRARIES( qedit ${QT_LIBRARIES} )
TARGET_LINK_LIBRARIES( qedit document-classes filesystem )
TARGET_LINK_LIBRARIES( qedit
  base
  base-qt
  base-server
  base-help
)

IF( ASPELL_FOUND )

  TARGET_LINK_LIBRARIES( qedit base-spellcheck )

ENDIF()

INSTALL( TARGETS qedit DESTINATION ${BIN_INSTALL_DIR} )
