TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \
  FileRecordProperties.h \
  FileSystemFrame.h \
  FileSystemHistory.h \
  FileSystemIcons.h \
  FileSystemModel.h \
  FileSystemThread.h

SOURCES = \
  FileSystemFrame.cpp \
  FileSystemHistory.cpp \
  FileSystemModel.cpp \
  FileSystemThread.cpp
