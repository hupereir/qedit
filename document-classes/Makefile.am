# $Id$

AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = document-classes.pro patterns.qrc

CXXFLAGS = @CXXFLAGS@

INCLUDES =  \
  -I$(qt_includes) \
  -I$(qt_includes)/Qt \
  -I$(qt_includes)/QtCore  \
  -I$(qt_includes)/QtXml \
  -I$(qt_includes)/QtGui \
  -I$(top_srcdir)/base \
  -I$(top_srcdir)/base-qt

noinst_HEADERS = \
  BlockDelimiter.h \
  BlockDelimiterDialog.h \
  BlockDelimiterList.h \
  BlockDelimiterModel.h \
  DocumentClass.h \
  DocumentClassConfiguration.h \
  DocumentClassDialog.h \
  DocumentClassManager.h \
  DocumentClassManagerDialog.h \
  DocumentClassModel.h \
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

noinst_LIBRARIES = libdocument-classes.a

libdocument_classes_a_SOURCES = \
  BlockDelimiter.cpp \
  BlockDelimiterDialog.cpp \
  BlockDelimiterList.cpp \
  BlockDelimiterModel.cpp \
  DocumentClass.cpp \
  DocumentClassConfiguration.cpp \
  DocumentClassDialog.cpp \
  DocumentClassManager.cpp \
  DocumentClassManagerDialog.cpp \
  DocumentClassModel.cpp \
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

nodist_libdocument_classes_a_SOURCES = \
  qrc_patterns.cpp \
  moc_BlockDelimiterList.cpp \
  moc_DocumentClassDialog.cpp \
  moc_DocumentClassManagerDialog.cpp \
  moc_HighlightStyleList.cpp \
  moc_HighlightPatternDialog.cpp \
  moc_HighlightPatternList.cpp \
  moc_HighlightPatternType.cpp \
  moc_IndentPatternList.cpp \
  moc_IndentPatternRuleList.cpp \
  moc_IndentPatternType.cpp \
  moc_TextMacroList.cpp \
  moc_TextMacroRuleList.cpp \
  moc_TextParenthesisList.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@

qrc_patterns.cpp: \
  ../patterns/c_patterns.xml \
  ../patterns/default_patterns.xml \
  ../patterns/diff_patterns.xml \
  ../patterns/fortran_patterns.xml \
  ../patterns/html_patterns.xml \
  ../patterns/latex_patterns.xml \
  ../patterns/makefile_patterns.xml \
  ../patterns/perl_patterns.xml \
  ../patterns/shell_patterns.xml \
  ../patterns/xml_patterns.xml
