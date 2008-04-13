TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \
DocumentClassDialog.h \
DocumentClass.h \
DocumentClassManagerDialog.h \
DocumentClassManager.h \
HighlightPattern.h \
HighlightPatternList.h \
HighlightPatternModel.h \
HighlightPatternOptions.h \
HighlightStyleDialog.h \
HighlightStyle.h \
HighlightStyleList.h \
HighlightStyleModel.h \
IndentPattern.h \
TextMacro.h \
TextParenthesis.h \
XmlDef.h

SOURCES = \
DocumentClass.cpp \
DocumentClassDialog.cpp \
DocumentClassManager.cpp \
DocumentClassManagerDialog.cpp \
HighlightPattern.cpp \
HighlightPatternList.cpp \
HighlightPatternModel.cpp \
HighlightPatternOptions.cpp \
HighlightStyle.cpp \
HighlightStyleDialog.cpp \
HighlightStyleList.cpp \
HighlightStyleModel.cpp \
IndentPattern.cpp \
KeywordPattern.cpp \
TextParenthesis.cpp
