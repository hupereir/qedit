# $Id$

########### X11 #########
IF( UNIX )

  FIND_PACKAGE( X11 REQUIRED )
  INCLUDE_DIRECTORIES( ${X11_X11_INCLUDE_PATH} )

ENDIF()

########### Qt4 configuration #########
IF( USE_QT5 )

  FIND_PACKAGE( Qt5Widgets REQUIRED )

ELSE()

  FIND_PACKAGE( Qt4 REQUIRED )
  SET( QT_USE_QTXML TRUE )
  SET( QT_USE_QTNETWORK TRUE )
  INCLUDE( ${QT_USE_FILE} )
  ADD_DEFINITIONS( ${QT_DEFINITIONS} )

ENDIF()

########### includes #########
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-qt)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-server)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-help)

IF( ASPELL_FOUND )
  INCLUDE_DIRECTORIES(${ASPELL_INCLUDE_DIR})
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
  CloseFilesDialog.cpp
  ConfigurationDialog.cpp
  Diff.cpp
  DocumentClassMenu.cpp
  DocumentClassToolBar.cpp
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
  CloseFilesDialog.h
  ConfigurationDialog.h
  Diff.h
  DocumentClassMenu.h
  DocumentClassToolBar.h
  FileCheck.h
  FileCheckDialog.h
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
  SaveAllDialog.h
  SessionFilesFrame.h
  SessionFilesModel.h
  TextDisplay.h
  TextView.h
  WindowServer.h
)

SET( qedit_RESOURCES pixmaps.qrc)

IF( USE_QT5 )

  QT5_WRAP_CPP( qedit_HEADERS_MOC ${qedit_HEADERS} )
  QT5_ADD_RESOURCES( qedit_RESOURCES_RCC ${qedit_RESOURCES})

ELSE()

  QT4_WRAP_CPP( qedit_HEADERS_MOC ${qedit_HEADERS} )
  QT4_ADD_RESOURCES( qedit_RESOURCES_RCC ${qedit_RESOURCES})

ENDIF()

ADD_APPLICATION_ICON( qedit_SOURCES ${CMAKE_SOURCE_DIR}/qedit )
ADD_DESKTOP_FILE( ${CMAKE_SOURCE_DIR}/qedit.desktop )
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

IF( USE_QT5 )

  QT5_USE_MODULES( qedit Network PrintSupport Widgets Xml )

ENDIF()
