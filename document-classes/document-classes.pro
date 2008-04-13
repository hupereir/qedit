TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \
DocumentClass.h \
DocumentClassManagerDialog.h \
DocumentClassManager.h \
HighlightPatternDialog.h \
HighlightPattern.h \
HighlightPatternList.h \
HighlightPatternModel.h \
HighlightPatternOptions.h \
HighlightPatternType.h \
HighlightStyleDialog.h \
HighlightStyle.h \
HighlightStyleList.h \
HighlightStyleModel.h \
IndentPattern.h \
PatternLocation.h \
TextMacro.h \
TextParenthesis.h \
XmlDef.h

SOURCES = \
DocumentClass.cpp \
DocumentClassDialog.cpp \
DocumentClassManager.cpp \
DocumentClassManagerDialog.cpp \
editDocumentClasses.cpp \
HighlightPattern.cpp \
HighlightPatternDialog.cpp \
HighlightPatternList.cpp \
HighlightPatternModel.cpp \
HighlightPatternOptions.cpp \
HighlightPatternType.cpp \
HighlightStyle.cpp \
HighlightStyleDialog.cpp \
HighlightStyleList.cpp \
HighlightStyleModel.cpp \
IndentPattern.cpp \
TextMacro.cpp \
TextParenthesis.cpp
