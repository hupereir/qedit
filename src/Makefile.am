AUTOMAKE_OPTIONS = foreign

if WITH_ASPELL 
INCLUDES = \
  -I$(aspell_includes) \
  -I$(qt_includes) \
  -I$(qt_includes)/Qt \
  -I$(qt_includes)/QtCore  \
  -I$(qt_includes)/QtNetwork  \
  -I$(qt_includes)/QtXml \
  -I$(qt_includes)/QtGui \
  -I$(top_srcdir)/base  \
  -I$(top_srcdir)/base-qt \
  -I$(top_srcdir)/base-server \
  -I$(top_srcdir)/base-spellcheck
else
INCLUDES = \
  -I$(qt_includes) \
  -I$(qt_includes)/Qt \
  -I$(qt_includes)/QtCore  \
  -I$(qt_includes)/QtNetwork  \
  -I$(qt_includes)/QtXml \
  -I$(qt_includes)/QtGui \
  -I$(top_srcdir)/base  \
  -I$(top_srcdir)/base-qt \
  -I$(top_srcdir)/base-server
endif

CXXFLAGS = @CXXFLAGS@

bin_PROGRAMS = qedit
noinst_LIBRARIES = libhighlight.a

if WITH_ASPELL 
libraries = \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libhighlight.a \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

libraries =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libhighlight.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

qedit_LDADD =  $(libraries)

noinst_HEADERS = \
  DocumentClass.h \
  HighlightBlockData.h \
  HighlightStyle.h \
  HighlightPattern.h \
  IndentPattern.h \
  KeywordPattern.h \
  RangePattern.h \
  TextBraces.h \
  TextHighlight.h \
  TextMacro.h

libhighlight_a_SOURCES = \
  HighlightStyle.cc \
  HighlightPattern.cc \
  KeywordPattern.cc \
  RangePattern.cc \
  TextHighlight.cc

qedit_SOURCES = \
  DocumentClass.cc \
  IndentPattern.cc \
  TextBraces.cc \
  TextMacro.cc

nodist_qedit_SOURCES =

### rule to generate qt meta objects
moc_%.cc: %.h
	$(qt_moc) $^ -o $@
