# $Id$

TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \
  FileInformationDialog.h \
  FileRecordProperties.h \
  FileSystemFrame.h \
  FileSystemHistory.h \
  FileSystemIcons.h \
  FileSystemModel.h \
  FileSystemThread.h \
  RemoveFilesDialog.h \
  RenameFileDialog.h


SOURCES = \
  FileInformationDialog.cpp \
  FileSystemFrame.cpp \
  FileSystemHistory.cpp \
  FileSystemModel.cpp \
  FileSystemThread.cpp \
  RemoveFilesDialog.cpp \
  RenameFileDialog.cpp
