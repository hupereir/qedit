TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \
  BlockDelimiter.h \
  DocumentClass.h \
  DocumentClassDialog.h \
  DocumentClassManager.h \
  DocumentClassManagerDialog.h \
  HighlightPattern.h \
  HighlightPatternDialog.h \
  HighlightPatternList.h \
  HighlightPatternModel.h \
  HighlightPatternOptions.h \
  HighlightPatternType.h \
  HighlightStyle.h \
  HighlightStyleList.h \
  HighlightStyleModel.h \
  HighlightStyleDialog.h \
  IndentPattern.h \
  PatternLocation.h \
  TextMacro.h \
  TextParenthesis.h \
  XmlDef.h

SOURCES = \
  BlockDelimiter.cpp \
  DocumentClass.cpp \
  DocumentClassDialog.cpp \
  DocumentClassManager.cpp \
  DocumentClassManagerDialog.cpp \
  HighlightPattern.cpp \
  HighlightPatternDialog.cpp \
  HighlightPatternList.cpp \
  HighlightPatternModel.cpp \
  HighlightPatternOptions.cpp \
  HighlightPatternType.cpp \
  HighlightStyle.cpp \
  HighlightStyleList.cpp \
  HighlightStyleModel.cpp \
  HighlightStyleDialog.cpp \
  IndentPattern.cpp \
  TextMacro.cpp \
  TextParenthesis.cpp
