########### Qt configuration #########
find_package(Qt5 COMPONENTS Widgets Network Xml PrintSupport REQUIRED )

if( WIN32 AND NOT USE_SHARED_LIBS )
  fix_win32_static_compilation()
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

include_directories(${CMAKE_SOURCE_DIR}/base-filesystem)
include_directories(${CMAKE_SOURCE_DIR}/document-classes)

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
  DocumentClassManagerDialog.cpp
  DocumentClassMenu.cpp
  DocumentClassModel.cpp
  DocumentClassToolBar.cpp
  FileCheck.cpp
  FileCheckDialog.cpp
  FileModifiedWidget.cpp
  FileReadOnlyWidget.cpp
  FileRemovedWidget.cpp
  FileSelectionDialog.cpp
  HtmlHelper.cpp
  MainWindow.cpp
  MenuBar.cpp
  SidePanelToolBar.cpp
  NewDocumentNameServer.cpp
  NewFileDialog.cpp
  PrintHelper.cpp
  ProgressDialog.cpp
  RecentFilesModel.cpp
  RecentFilesWidget.cpp
  ReplaceWidget.cpp
  RestoreSessionDialog.cpp
  SaveAllDialog.cpp
  SessionFilesModel.cpp
  SessionFilesView.cpp
  SessionFilesWidget.cpp
  SidePanelWidget.cpp
  TextDisplay.cpp
  TextView.cpp
  WindowServer.cpp
  main.cpp
)

set(qedit_RESOURCES pixmaps.qrc)

add_application_icon(qedit_SOURCES ${CMAKE_SOURCE_DIR}/qedit)
add_desktop_file(${CMAKE_SOURCE_DIR}/qedit)
add_platform_executable(qedit
  ${QEDIT_VERSION}
  ${qedit_SOURCES}
  ${qedit_RESOURCES}
)

target_link_libraries(qedit document-classes)
target_link_libraries(qedit
  base
  base-qt
  base-server
  base-help
  base-filesystem
)

if(ASPELL_FOUND)
  target_link_libraries(qedit base-spellcheck)
endif()

target_link_libraries(qedit Qt5::Network Qt5::PrintSupport Qt5::Widgets Qt5::Xml)
install(TARGETS qedit DESTINATION ${BIN_INSTALL_DIR})

