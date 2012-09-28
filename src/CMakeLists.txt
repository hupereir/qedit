PROJECT( QEDIT_SRC )
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

########### Qt4 configuration #########
FIND_PACKAGE( Qt4 REQUIRED )
SET( QT_USE_QTXML TRUE )

INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} )


########### X11 #########
IF( NOT Q_WS_WIN )

  FIND_PACKAGE( X11 REQUIRED )
  INCLUDE_DIRECTORIES( ${X11_X11_INCLUDE_PATH} )

ENDIF()

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
SET( qedit_lib_SOURCES
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
)

SET( qedit_lib_HEADERS
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

QT4_WRAP_CPP( qedit_lib_HEADERS_MOC ${qedit_lib_HEADERS} )

ADD_LIBRARY( qedit_lib STATIC
  ${qedit_lib_SOURCES}
  ${qedit_lib_HEADERS_MOC}
)

########### next target ###############
SET( qedit_SOURCES qedit.cpp )
SET( qedit_RESOURCES pixmaps.qrc)
QT4_ADD_RESOURCES( qedit_RESOURCES_RCC ${qedit_RESOURCES})

IF( Q_WS_WIN )

  SET( qedit_RC qedit.rc )
  ADD_EXECUTABLE( qedit WIN32 ${qedit_SOURCES} ${qedit_RESOURCES_RCC} ${qedit_RC} )

ELSE()

  ADD_EXECUTABLE( qedit ${qedit_SOURCES} ${qedit_RESOURCES_RCC} )
  TARGET_LINK_LIBRARIES( qedit ${X11_X11_LIB} )

ENDIF()

TARGET_LINK_LIBRARIES( qedit ${QT_LIBRARIES} )
TARGET_LINK_LIBRARIES( qedit
  qedit_lib
  document-classes
  filesystem
)

TARGET_LINK_LIBRARIES( qedit
  base
  base-qt
  base-server
  base-help
)

IF( ASPELL_FOUND )

  TARGET_LINK_LIBRARIES( qedit base-spellcheck )

ENDIF()

INSTALL( TARGETS qedit DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
