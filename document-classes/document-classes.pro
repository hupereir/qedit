TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt
DEPENDPATH += . ../base ../base-qt

HEADERS = \
  BlockDelimiter.h \
  BlockDelimiterDialog.h \
  BlockDelimiterList.h \
  BlockDelimiterModel.h \
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
  TextMacroDialog.h \
  TextMacroList.h \
  TextMacroModel.h \
  TextMacroRuleDialog.h \
  TextMacroRuleList.h \
  TextMacroRuleModel.h \
  TextParenthesis.h \
  TextParenthesisDialog.h \
  TextParenthesisList.h \
  TextParenthesisModel.h \
  XmlDef.h

SOURCES = \
  BlockDelimiter.cpp \
  BlockDelimiterDialog.cpp \
  BlockDelimiterList.cpp \
  BlockDelimiterModel.cpp \
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
  TextMacroList.cpp \
  TextMacroModel.cpp \
  TextMacroDialog.cpp \
  TextMacroRuleDialog.cpp \
  TextMacroRuleList.cpp \
  TextMacroRuleModel.cpp \
  TextParenthesis.cpp \
  TextParenthesisDialog.cpp \
  TextParenthesisList.cpp \
  TextParenthesisModel.cpp
