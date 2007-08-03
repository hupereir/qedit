
CONFIG += qt
QT += xml

TARGET = Top
VERSION = 2.0
DESTDIR = $$(HOME)/bin/.

INCLUDEPATH = . ../base ../base-qt ../server
DEPENDPATH += . ../base ../base-qt ../server

LIBS += \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a \
  ../server/libserver.a

POST_TARGETDEPS = \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a \
  ../server/libserver.a

SOURCES = \
  MainFrame.cc \
  Menu.cc \
  Top.cc \
  TopWidget.cc

HEADERS = \
  MainFrame.h \
  Menu.h \
  TopWidget.h
