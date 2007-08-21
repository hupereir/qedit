TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \  
  DocumentClass.h \
  DocumentClassManager.h \
  HighlightPattern.h \
  HighlightStyle.h \
  IndentPattern.h \
  KeywordPattern.h \
  RangePattern.h \
  TextMacro.h \
  TextParenthesis.h

SOURCES = \
  DocumentClass.cpp \
  DocumentClassManager.cpp \
  HighlightPattern.cpp \
  HighlightStyle.cpp \
  IndentPattern.cpp \
  KeywordPattern.cpp \
  RangePattern.cpp \
  TextMacro.cpp \
  TextParenthesis.cpp

