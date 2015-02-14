# $Id$

########### Qt configuration #########
if(USE_QT5)

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5Xml REQUIRED)
  find_package(Qt5PrintSupport REQUIRED)

else()

  find_package(Qt4 REQUIRED)
  set(QT_USE_QTXML TRUE)
  set(QT_USE_QTNETWORK TRUE)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})

endif()

########### includes #########
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/base-qt)
include_directories(${CMAKE_SOURCE_DIR}/base-server)
include_directories(${CMAKE_SOURCE_DIR}/base-help)

if(ASPELL_FOUND)
  include_directories(${ASPELL_INCLUDE_DIR})
  include_directories(${CMAKE_SOURCE_DIR}/base-spellcheck)
endif()

include_directories(${CMAKE_SOURCE_DIR}/document-classes)
include_directories(${CMAKE_SOURCE_DIR}/filesystem)

########### configuration files ###############
configure_file(Config.h.cmake Config.h)
configure_file(SystemOptions.h.cmake SystemOptions.h)

########### next target ###############
set(qedit_SOURCES
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
  ProgressDialog.cpp
  RecentFilesFrame.cpp
  ReplaceWidget.cpp
  RestoreSessionDialog.cpp
  SaveAllDialog.cpp
  SessionFilesFrame.cpp
  SessionFilesModel.cpp
  SessionFilesView.cpp
  TextDisplay.cpp
  TextView.cpp
  WindowServer.cpp
  main.cpp
)

set(qedit_RESOURCES pixmaps.qrc)

if(USE_QT5)

  qt5_add_resources(qedit_RESOURCES_RCC ${qedit_RESOURCES})

else()

  qt4_add_resources(qedit_RESOURCES_RCC ${qedit_RESOURCES})

endif()

add_application_icon(qedit_SOURCES ${CMAKE_SOURCE_DIR}/qedit)
add_desktop_file(${CMAKE_SOURCE_DIR}/qedit)
add_platform_executable(qedit
  ${QEDIT_VERSION}
  ${qedit_SOURCES}
  ${qedit_RESOURCES_RCC}
)

target_link_libraries(qedit ${QT_LIBRARIES})
target_link_libraries(qedit document-classes filesystem)
target_link_libraries(qedit
  base
  base-qt
  base-server
  base-help
)

if(ASPELL_FOUND)

  target_link_libraries(qedit base-spellcheck)

endif()

install(TARGETS qedit DESTINATION ${BIN_INSTALL_DIR})

if(USE_QT5)

  qt5_use_modules(qedit Network PrintSupport Widgets Xml)

endif()
