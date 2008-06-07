# $Id$

TEMPLATE = app
CONFIG += qt release
QT += xml network

TARGET = qedit

VERSION = 1.1
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
  AskForSaveDialog.h \
  AutoSave.h \
  AutoSaveThread.h \
  CollapsedBlockData.h \
  BlockDelimiterSegment.h \
  BlockDelimiterWidget.h \
  BlockMarker.h \
  ConfigurationDialog.h \
  Diff.h \
  DefaultOptions.h \
  EditFrame.h \
  ExitDialog.h \
  FileInfoDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
  HtmlUtil.h \
  Icons.h \
  LineNumberWidget.h \
  MainFrame.h \
  Menu.h \
  NewFileDialog.h \
  ParenthesisHighlight.h \
  PrintDialog.h \
  TextDisplay.h \
  TextHighlight.h \
  TextIndent.h \
  TextView.h \
  WindowTitle.h \
  XmlDef.h
 
SOURCES = \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  BlockDelimiterWidget.cpp \
  CollapsedBlockData.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  EditFrame.cpp \
  ExitDialog.cpp \
  FileInfoDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  HighlightBlockData.cpp \
  HighlightBlockFlags.cpp \
  HtmlUtil.cpp \
  LineNumberWidget.cpp \
  MainFrame.cpp \
  Menu.cpp \
  NewFileDialog.cpp \
  ParenthesisHighlight.cpp \
  PrintDialog.cpp \
  qedit.cpp \
  TextDisplay.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextView.cpp
