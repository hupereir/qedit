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

##################
# shared libraries
if ENABLE_SHARED
if WITH_ASPELL
libraries = \
  -L$(libdir) -lbase-spellcheck -lbase-server -lbase-help \
  libqedit.a \
  ../filesystem/libfilesystem.a \
  ../document-classes/libdocument-classes.a

else

libraries =  \
  -L$(libdir) -lbase-server -lbase-help \
  libqedit.a \
  ../filesystem/libfilesystem.a \
  ../document-classes/libdocument-classes.a

endif

else

##################
# static libraries
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
endif

#################
# headers
noinst_HEADERS = \
  Application.h \
  AskForSaveDialog.h \
  AutoSave.h \
  AutoSaveThread.h \
  ConfigurationDialog.h \
  Diff.h \
  DefaultOptions.h \
  DocumentClassMenu.h \
  DocumentClassToolBar.h \
  CloseFilesDialog.h \
  FileCheck.h \
  FileCheckDialog.h \
  FileModifiedDialog.h \
  FileRemovedDialog.h \
  FileSelectionDialog.h \
  HelpText.h \
  Icons.h \
  MainWindow.h \
  Menu.h \
  NavigationFrame.h \
  NavigationToolBar.h \
  NewDocumentNameServer.h \
  NewFileDialog.h \
  ProgressDialog.h \
  RecentFilesFrame.h \
  ReplaceDialog.h \
  SaveAllDialog.h \
  SessionFilesFrame.h \
  SessionFilesModel.h \
  TextDisplay.h \
  TextView.h \
  WindowServer.h \
  WindowTitle.h

#####################
# qedit library sources
libqedit_a_SOURCES = \
  Application.cpp \
  AskForSaveDialog.cpp \
  AutoSave.cpp \
  AutoSaveThread.cpp \
  ConfigurationDialog.cpp \
  Diff.cpp \
  DocumentClassMenu.cpp \
  DocumentClassToolBar.cpp \
  CloseFilesDialog.cpp \
  FileCheck.cpp \
  FileCheckDialog.cpp \
  FileModifiedDialog.cpp \
  FileRemovedDialog.cpp \
  FileSelectionDialog.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NavigationFrame.cpp \
  NavigationToolBar.cpp \
  NewDocumentNameServer.cpp \
  NewFileDialog.cpp \
  ReplaceDialog.cpp \
  RecentFilesFrame.cpp \
  SaveAllDialog.cpp \
  SessionFilesFrame.cpp \
  SessionFilesModel.cpp \
  TextDisplay.cpp \
  TextView.cpp \
  WindowServer.cpp

nodist_libqedit_a_SOURCES = \
  qrc_pixmaps.cpp \
  moc_Application.cpp \
  moc_AskForSaveDialog.cpp \
  moc_AutoSave.cpp \
  moc_Diff.cpp \
  moc_DocumentClassMenu.cpp \
  moc_DocumentClassToolBar.cpp \
	 moc_FileCheck.cpp \
  moc_FileModifiedDialog.cpp \
  moc_FileRemovedDialog.cpp \
  moc_FileSelectionDialog.cpp \
  moc_MainWindow.cpp \
  moc_Menu.cpp \
  moc_NavigationFrame.cpp \
  moc_NavigationToolBar.cpp \
  moc_NewFileDialog.cpp \
  moc_ProgressDialog.cpp \
  moc_RecentFilesFrame.cpp \
  moc_ReplaceDialog.cpp \
  moc_SessionFilesFrame.cpp \
  moc_SessionFilesModel.cpp \
  moc_TextDisplay.cpp \
  moc_TextView.cpp \
  moc_WindowServer.cpp

#######################
# qedit
qedit_LDADD =  $(libraries)
qedit_SOURCES = qedit.cpp

#######################
# editDocumentClasses
editDocumentClasses_LDADD =  $(libraries)
editDocumentClasses_SOURCES = editDocumentClasses.cpp

# clean
clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

### document class patterns
qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
