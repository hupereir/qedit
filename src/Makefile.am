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
  -I$(top_srcdir)/document-classes \
  -I$(top_srcdir)/filesystem
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
  -I$(top_srcdir)/document-classes \
  -I$(top_srcdir)/filesystem
endif

CXXFLAGS = @CXXFLAGS@

noinst_LIBRARIES = libqedit.a
bin_PROGRAMS = qedit editDocumentClasses

if WITH_ASPELL 
libraries = \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libqedit.a \
  ../filesystem/libfilesystem.a \
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
  ../filesystem/libfilesystem.a \
  ../document-classes/libdocument-classes.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

qedit_LDADD =  $(libraries)
editDocumentClasses_LDADD =  $(libraries)

noinst_HEADERS = \
  Application.h \
  AskForSaveDialog.h \
  AutoSave.h \
  AutoSaveThread.h \
  BlockDelimiterSegment.h \
  BlockDelimiterDisplay.h \
  BlockMarker.h \
  CollapsedBlockData.h \
  ConfigurationDialog.h \
  Diff.h \
  DefaultOptions.h \
  CloseFilesDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  HighlightBlockData.h \
  HighlightBlockFlags.h \
  Icons.h \
  MainWindow.h \
  Menu.h \
  NavigationFrame.h \
  NavigationToolBar.h \
  NewDocumentNameServer.h \
  NewFileDialog.h \
  ParenthesisHighlight.h \
  PrintDialog.h \
  RecentFilesFrame.h \
  ReplaceDialog.h \
  SaveAllDialog.h \
  SessionFilesFrame.h \
  SessionFilesModel.h \
  TextDisplay.h \
  TextHighlight.h \
  TextIndent.h \
  TextView.h \
  WindowServer.h \
  WindowTitle.h

libqedit_a_SOURCES = \
  Application.cpp \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  BlockDelimiterDisplay.cpp \
  CollapsedBlockData.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  CloseFilesDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  HighlightBlockData.cpp \
  HighlightBlockFlags.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NavigationFrame.cpp \
  NavigationToolBar.cpp \
  NewDocumentNameServer.cpp \
  NewFileDialog.cpp \
  ParenthesisHighlight.cpp \
  PrintDialog.cpp \
  ReplaceDialog.cpp \
  RecentFilesFrame.cpp \
  SaveAllDialog.cpp \
  SessionFilesFrame.cpp \
  SessionFilesModel.cpp \
  TextDisplay.cpp \
  TextHighlight.cpp \
  TextIndent.cpp \
  TextView.cpp \
  WindowServer.cpp

nodist_libqedit_a_SOURCES = \
  qrc_pixmaps.cpp \
  moc_Application.cpp \
  moc_AskForSaveDialog.cpp \
  moc_AutoSave.cpp \
  moc_BlockDelimiterDisplay.cpp \
  moc_ConfigurationDialog.cpp \
  moc_Diff.cpp \
  moc_FileModifiedDialog.cpp \
  moc_FileRemovedDialog.cpp \
  moc_FileSelectionDialog.cpp \
  moc_MainWindow.cpp \
  moc_Menu.cpp \
  moc_NavigationToolBar.cpp \
  moc_NewFileDialog.cpp \
  moc_ParenthesisHighlight.cpp \
  moc_PrintDialog.cpp \
  moc_RecentFilesFrame.cpp \
	moc_ReplaceDialog.cpp \
  moc_SessionFilesFrame.cpp \
  moc_SessionFilesModel.cpp \
  moc_TextDisplay.cpp \
  moc_TextHighlight.cpp \
  moc_TextIndent.cpp \
  moc_TextView.cpp \
  moc_WindowServer.cpp

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
