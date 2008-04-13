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
  TextMacro.h \
  TextParenthesis.h \
  XmlDef.h

noinst_LIBRARIES = libdocument-classes.a

libdocument_classes_a_SOURCES = \
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

nodist_libdocument_classes_a_SOURCES = \
  qrc_patterns.cpp \
  moc_DocumentClassDialog.cpp \
  moc_DocumentClassManagerDialog.cpp \
  moc_HighlightStyleList.cpp \
  moc_HighlightPatternDialog.cpp \
  moc_HighlightPatternList.cpp \
  moc_HighlightPatternType.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
