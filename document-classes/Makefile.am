AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = document-classes.pro

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
  DocumentClassManager.h \
  HighlightPattern.h \
  HighlightStyle.h \
  IndentPattern.h \
  KeywordPattern.h \
  RangePattern.h \
  TextMacro.h \
  TextParenthesis.h \
  XmlDef.h

noinst_LIBRARIES = libdocument-classes.a

libdocument_classes_a_SOURCES = \
  DocumentClass.cpp \
  DocumentClassManager.cpp \
  HighlightPattern.cpp \
  HighlightStyle.cpp \
  IndentPattern.cpp \
  KeywordPattern.cpp \
  RangePattern.cpp \
  TextMacro.cpp \
  TextParenthesis.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
