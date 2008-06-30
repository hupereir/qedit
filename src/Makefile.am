# $Id$

AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = src.pro qedit.rc pixmaps.qrc 

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
  -I$(top_srcdir)/base-help \
  -I$(top_srcdir)/base-server \
  -I$(top_srcdir)/base-spellcheck \
  -I$(top_srcdir)/document-classes
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
  -I$(top_srcdir)/base-help \
  -I$(top_srcdir)/base-server \
  -I$(top_srcdir)/document-classes
endif

CXXFLAGS = @CXXFLAGS@

noinst_LIBRARIES = libqedit.a
bin_PROGRAMS = qedit editDocumentClasses

if WITH_ASPELL 
libraries = \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libqedit.a \
  ../document-classes/libdocument-classes.a \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

libraries =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libqedit.a \
  ../document-classes/libdocument-classes.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

qedit_LDADD =  $(libraries)
editDocumentClasses_LDADD =  $(libraries)

noinst_HEADERS = \
  AskForSaveDialog.h \
  AutoSave.h \
  AutoSaveThread.h \
  BlockDelimiterSegment.h \
  BlockDelimiterWidget.h \
  BlockMarker.h \
  CollapsedBlockData.h \
  ConfigurationDialog.h \
  Diff.h \
  DefaultOptions.h \
  EditFrame.h \
  ExitDialog.h \
  FileInfoDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
  HtmlUtil.h \
  Icons.h \
  LineNumberWidget.h \
  MainFrame.h \
  Menu.h \
  NewFileDialog.h \
  ParenthesisHighlight.h \
  PrintDialog.h \
  TextDisplay.h \
  TextHighlight.h \
  TextIndent.h \
  TextView.h \
  WindowTitle.h

libqedit_a_SOURCES = \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  BlockDelimiterWidget.cpp \
  CollapsedBlockData.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  EditFrame.cpp \
  ExitDialog.cpp \
  FileInfoDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  HighlightBlockData.cpp \
  HighlightBlockFlags.cpp \
  HtmlUtil.cpp \
  LineNumberWidget.cpp \
  MainFrame.cpp \
  Menu.cpp \
  NewFileDialog.cpp \
  ParenthesisHighlight.cpp \
  PrintDialog.cpp \
  TextDisplay.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextView.cpp

nodist_libqedit_a_SOURCES = \
  qrc_pixmaps.cpp \
  moc_AskForSaveDialog.cpp \
  moc_AutoSave.cpp \
  moc_BlockDelimiterWidget.cpp \
  moc_ConfigurationDialog.cpp \
  moc_Diff.cpp \
  moc_EditFrame.cpp \
  moc_FileModifiedDialog.cpp \
  moc_FileRemovedDialog.cpp \
  moc_FileSelectionDialog.cpp \
  moc_LineNumberWidget.cpp \
  moc_MainFrame.cpp \
  moc_Menu.cpp \
  moc_NewFileDialog.cpp \
  moc_ParenthesisHighlight.cpp \
  moc_PrintDialog.cpp \
  moc_TextDisplay.cpp \
  moc_TextHighlight.cpp \
  moc_TextIndent.cpp \
  moc_TextView.cpp

qedit_SOURCES = qedit.cpp
editDocumentClasses_SOURCES = editDocumentClasses.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

### document class patterns
qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
