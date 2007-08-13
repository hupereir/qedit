AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = src.pro

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
  Config.h \
  ConfigurationDialog.h \
  DefaultOptions.h \
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
  ViewHtmlDialog.h \
  WindowTitle.h \
  XmlDef.h

qedit_SOURCES = \
  AskForSaveDialog.cc \
  AutoSave.cc \
  AutoSaveThread.cc \
  ConfigurationDialog.cc \
  Diff.cc \
  DocumentClass.cc \
  DocumentClassDialog.cc \
  DocumentClassManager.cc \
  EditFrame.cc \
  ExitDialog.cc \
  FileInfoDialog.cc \
  FileModifiedDialog.cc \
  FileRemovedDialog.cc \
  FileSelectionDialog.cc \
  HighlightBlockData.cc \
  HighlightPattern.cc \
  HighlightStyle.cc \
  HtmlUtil.cc \
  IndentPattern.cc \
  KeywordPattern.cc \
  MainFrame.cc \
  Menu.cc \
  NewFileDialog.cc \
  ParenthesisHighlight.cc \
  PrintDialog.cc \
  QEdit.cc \
  RangePattern.cc \
  TextParenthesis.cc \
  TextDisplay.cc \
  TextHighlight.cc \
  TextIndent.cc \
  TextMacro.cc \
  ViewHtmlDialog.cc

nodist_qedit_SOURCES = \
  moc_AskForSaveDialog.cc \
  moc_AutoSave.cc \
  moc_Diff.cc \
  moc_DocumentClassDialog.cc \
  moc_EditFrame.cc \
  moc_FileModifiedDialog.cc \
  moc_FileRemovedDialog.cc \
  moc_FileSelectionDialog.cc \
  moc_MainFrame.cc \
  moc_Menu.cc \
  moc_NewFileDialog.cc \
  moc_ParenthesisHighlight.cc \
  moc_PrintDialog.cc \
  moc_TextDisplay.cc \
  moc_TextIndent.cc \
  moc_ViewHtmlDialog.cc

### rule to generate qt meta objects
moc_%.cc: %.h
	$(qt_moc) $^ -o $@
