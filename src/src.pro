# $Id$

TEMPLATE = app
CONFIG += qt release
QT += xml network

TARGET = qedit

VERSION = 1.3
DEFINES += VERSION=\"$$VERSION\"

INCLUDEPATH = . ../base ../base-qt ../base-help ../base-server ../document-classes ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-help ../base-server ../document-classes ../extra-includes

LIBS += \
  ../document-classes/libdocument-classes.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a


POST_TARGETDEPS = \
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
  ExitDialog.h \
  FileInfoDialog.h \
  FileModifiedDialog.h \
  FileRecordProperties.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  FileSystemFrame.h \
  FileSystemHistory.h \
  FileSystemModel.h \
  FileSystemThread.h \
  HelpText.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
  HtmlUtil.h \
  Icons.h \
  MainWindow.h \
  Menu.h \
  NavigationFrame.h \
  NewFileDialog.h \
  ParenthesisHighlight.h \
  PrintDialog.h \
  RecentFilesFrame.h \
  SaveAllDialog.h \
  SessionFilesFrame.h \
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
  ExitDialog.cpp \
  FileInfoDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  FileSystemFrame.cpp \
  FileSystemHistory.cpp \
  FileSystemModel.cpp \
  FileSystemThread.cpp \
  HighlightBlockData.cpp \
  HighlightBlockFlags.cpp \
  HtmlUtil.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NavigationFrame.cpp \
  NewFileDialog.cpp \
  ParenthesisHighlight.cpp \
  PrintDialog.cpp \
  RecentFilesFrame.cpp \
  SaveAllDialog.cpp \
  SessionFilesFrame.cpp \
  TextDisplay.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextView.cpp \
  WindowServer.cpp \
  qedit.cpp
