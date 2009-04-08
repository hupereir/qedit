TEMPLATE = app
TARGET = qedit

CONFIG += qt release
QT += xml network

win32 {
  DEFINES += QT_STATIC
  DESTDIR = "C:/Program Files" 
}

VERSION = 2.1.0
DEFINES += VERSION=\\\"$$VERSION\\\"

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
  BlockDelimiterSegment.h \
  BlockDelimiterDisplay.h \
  BlockMarker.h \
  CollapsedBlockData.h \
  ConfigurationDialog.h \
  Diff.h \
  DefaultOptions.h \
  CloseFilesDialog.h \
  FileCheck.h \
  FileCheckDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
  Icons.h \
  MainWindow.h \
  Menu.h \
  NavigationFrame.h \
  NavigationToolBar.h \
  NewDocumentNameServer.h \
  NewFileDialog.h \
  ParenthesisHighlight.h \
  PrintDialog.h \
  ProgressDialog.h \
  RecentFilesFrame.h \
  ReplaceDialog.h \
  SaveAllDialog.h \
  SessionFilesFrame.h \
  SessionFilesModel.h \
  TextDisplay.h \
  TextHighlight.h \
  TextIndent.h \
  TextView.h \
  WindowServer.h \
  WindowTitle.h
 
SOURCES = \
  Application.cpp \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  BlockDelimiterDisplay.cpp \
  CollapsedBlockData.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  CloseFilesDialog.cpp \
  FileCheck.cpp \
  FileCheckDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  HighlightBlockData.cpp \
  HighlightBlockFlags.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NavigationFrame.cpp \
  NavigationToolBar.cpp \
  NewDocumentNameServer.cpp \
  NewFileDialog.cpp \
  ParenthesisHighlight.cpp \
  PrintDialog.cpp \
  RecentFilesFrame.cpp \
  ReplaceDialog.cpp \
  SaveAllDialog.cpp \
  SessionFilesFrame.cpp \
  SessionFilesModel.cpp \
  TextDisplay.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextView.cpp \
  WindowServer.cpp \
  qedit.cpp
