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
HighlightPatternOptions.h \
HighlightStyleDialog.h \
HighlightStyle.h \
HighlightStyleItem.h \
HighlightStyleList.h \
IndentPattern.h \
KeywordPattern.h \
RangePattern.h \
TextMacro.h \
TextParenthesis.h \
XmlDef.h

SOURCES = \
DocumentClass.cpp \
DocumentClassDialog.cpp \
DocumentClassManager.cpp \
DocumentClassManagerDialog.cpp \
HighlightPattern.cpp \
HighlightPatternOptions.cpp \
HighlightStyle.cpp \
HighlightStyleDialog.cpp \
HighlightStyleList.cpp \
IndentPattern.cpp \
KeywordPattern.cpp \
RangePattern.cpp \
TextMacro.cpp \
TextParenthesis.cpp
