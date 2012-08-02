# $Id$

TEMPLATE = lib
CONFIG = staticlib qt
QT += xml

INCLUDEPATH += ../base ../base-qt ../extra-includes
DEPENDPATH += . ../base ../base-qt ../extra-includes

HEADERS = \
  BlockDelimiter.h \
  BlockDelimiterDialog.h \
  BlockDelimiterDisplay.h \
  BlockDelimiterList.h \
  BlockDelimiterModel.h \
  BlockDelimiterSegment.h \
  BlockMarker.h \
  CollapsedBlockData.h \
  DocumentClass.h \
  DocumentClassConfiguration.h \
  DocumentClassDialog.h \
  DocumentClassIcons.h \
  DocumentClassManager.h \
  DocumentClassManagerDialog.h \
  DocumentClassModel.h \
  DocumentClassModifiedDialog.h \
  DocumentClassPreview.h \
  DocumentClassTextEditor.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
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
  IndentPatternDialog.h \
  IndentPatternList.h \
  IndentPatternModel.h \
  IndentPatternRuleDialog.h \
  IndentPatternRuleList.h \
  IndentPatternRuleModel.h \
  IndentPatternType.h \
  ParenthesisHighlight.h \
  PatternLocation.h \
  TextHighlight.h \
  TextIndent.h \
  TextMacro.h \
  TextMacroDialog.h \
  TextMacroList.h \
  TextMacroMenu.h \
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
  BlockDelimiterDisplay.cpp \
  BlockDelimiterList.cpp \
  BlockDelimiterModel.cpp \
  CollapsedBlockData.cpp \
  DocumentClass.cpp \
  DocumentClassConfiguration.cpp \
  DocumentClassDialog.cpp \
  DocumentClassManager.cpp \
  DocumentClassManagerDialog.cpp \
  DocumentClassModel.cpp \
  DocumentClassModifiedDialog.cpp \
  DocumentClassPreview.cpp \
  DocumentClassTextEditor.cpp \
  HighlightBlockData.cpp \
  HighlightBlockFlags.cpp \
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
  IndentPatternDialog.cpp \
  IndentPatternList.cpp \
  IndentPatternModel.cpp \
  IndentPatternRuleDialog.cpp \
  IndentPatternRuleList.cpp \
  IndentPatternRuleModel.cpp \
  IndentPatternType.cpp \
  ParenthesisHighlight.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextMacro.cpp \
  TextMacroList.cpp \
  TextMacroMenu.cpp \
  TextMacroModel.cpp \
  TextMacroDialog.cpp \
  TextMacroRuleDialog.cpp \
  TextMacroRuleList.cpp \
  TextMacroRuleModel.cpp \
  TextParenthesis.cpp \
  TextParenthesisDialog.cpp \
  TextParenthesisList.cpp \
  TextParenthesisModel.cpp
