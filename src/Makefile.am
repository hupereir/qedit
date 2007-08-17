AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = src.pro pixmaps.qrc patterns.qrc

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

if WITH_ASPELL 
libraries = \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

libraries =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

qedit_LDADD =  $(libraries)

noinst_HEADERS = \
  AskForSaveDialog.h \
  AutoSave.h \
  AutoSaveThread.h \
  ConfigurationDialog.h \
  Diff.h \
  DocumentClassDialog.h \
  DocumentClass.h \
  DocumentClassManager.h \
  EditFrame.h \
  ExitDialog.h \
  FileInfoDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
  HighlightPattern.h \
  HighlightStyle.h \
  HtmlUtil.h \
  Icons.h \
  IndentPattern.h \
  KeywordPattern.h \
  MainFrame.h \
  Menu.h \
  NewFileDialog.h \
  ParenthesisHighlight.h \
  PrintDialog.h \
  RangePattern.h \
  TextParenthesis.h \
  TextDisplay.h \
  TextHighlight.h \
  TextIndent.h \
  TextMacro.h \
  WindowTitle.h \
  XmlDef.h

qedit_SOURCES = \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  DocumentClass.cpp \
  DocumentClassDialog.cpp \
  DocumentClassManager.cpp \
  EditFrame.cpp \
  ExitDialog.cpp \
  FileInfoDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  HighlightBlockData.cpp \
  HighlightPattern.cpp \
  HighlightStyle.cpp \
  HtmlUtil.cpp \
  IndentPattern.cpp \
  KeywordPattern.cpp \
  MainFrame.cpp \
  Menu.cpp \
  NewFileDialog.cpp \
  ParenthesisHighlight.cpp \
  PrintDialog.cpp \
  QEdit.cpp \
  RangePattern.cpp \
  TextParenthesis.cpp \
  TextDisplay.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextMacro.cpp

nodist_qedit_SOURCES = \
  qrc_pixmaps.cpp \
  qrc_patterns.cpp \
  moc_AskForSaveDialog.cpp \
  moc_AutoSave.cpp \
  moc_Diff.cpp \
  moc_DocumentClassDialog.cpp \
  moc_EditFrame.cpp \
  moc_FileModifiedDialog.cpp \
  moc_FileRemovedDialog.cpp \
  moc_FileSelectionDialog.cpp \
  moc_MainFrame.cpp \
  moc_Menu.cpp \
  moc_NewFileDialog.cpp \
  moc_ParenthesisHighlight.cpp \
  moc_PrintDialog.cpp \
  moc_TextDisplay.cpp \
  moc_TextIndent.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
