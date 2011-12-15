TEMPLATE = app
TARGET = qedit

CONFIG += qt release
QT += xml network

VERSION = 2.4.0
DEFINES += VERSION=\\\"$$VERSION\\\"

win32 {

  DEFINES += QT_STATIC
  DESTDIR = "C:\Program Files"
  QMAKE_LFLAGS += -static-libgcc -static-libstdc++

  # this is needed to copy target into relevant versioned name
  exists( \"$$DESTDIR\\upx.exe\" ) {

     # if available, use upx to compress the file
     version.commands = "\"$$DESTDIR\\upx.exe\" -9 -f -o \"$$DESTDIR\\$$TARGET-"$$VERSION".exe\""  "\"$$DESTDIR\\"$$TARGET".exe\"

  } else {

     # simple copy
     version.commands = @copy "\"$$DESTDIR\\"$$TARGET".exe\" \"$$DESTDIR\\$$TARGET-"$$VERSION".exe\""

  }

  # add to Post targets
  QMAKE_EXTRA_TARGETS += version
  QMAKE_POST_LINK += $$version.commands

}

INCLUDEPATH = . ../base ../base-qt ../base-help ../base-server ../document-classes ../extra-includes ../filesystem
DEPENDPATH += . ../base ../base-qt ../base-help ../base-server ../document-classes ../extra-includes ../filesystem

LIBS += \
  ../filesystem/libfilesystem.a \
  ../document-classes/libdocument-classes.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a


POST_TARGETDEPS = \
  ../filesystem/libfilesystem.a \
  ../document-classes/libdocument-classes.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

RESOURCES = pixmaps.qrc ../base-qt/basePixmaps.qrc ../document-classes/patterns.qrc
RC_FILE = qedit.rc

HEADERS = \
  Application.h \
  AskForSaveDialog.h \
  AutoSave.h \
  AutoSaveThread.h \
  ConfigurationDialog.h \
  Diff.h \
  DefaultOptions.h \
  DocumentClassMenu.h \
  DocumentClassToolBar.h \
  CloseFilesDialog.h \
  FileCheck.h \
  FileCheckDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  Icons.h \
  MainWindow.h \
  Menu.h \
  NavigationFrame.h \
  NavigationToolBar.h \
  NewDocumentNameServer.h \
  NewFileDialog.h \
  PrintHelper.h \
  PrintPreviewDialog.h \
  ProgressDialog.h \
  RecentFilesFrame.h \
  ReplaceDialog.h \
  SaveAllDialog.h \
  SessionFilesFrame.h \
  SessionFilesModel.h \
  TextDisplay.h \
  TextView.h \
  WindowServer.h \
  WindowTitle.h

SOURCES = \
  Application.cpp \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  DocumentClassMenu.cpp \
  DocumentClassToolBar.cpp \
  CloseFilesDialog.cpp \
  FileCheck.cpp \
  FileCheckDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NavigationFrame.cpp \
  NavigationToolBar.cpp \
  NewDocumentNameServer.cpp \
  NewFileDialog.cpp \
  PrintHelper.cpp \
  PrintPreviewDialog.cpp \
  ReplaceDialog.cpp \
  RecentFilesFrame.cpp \
  SaveAllDialog.cpp \
  SessionFilesFrame.cpp \
  SessionFilesModel.cpp \
  TextDisplay.cpp \
  TextView.cpp \
  WindowServer.cpp \
  qedit.cpp
