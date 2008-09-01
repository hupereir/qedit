# $Id$

AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = filesystem.pro

INCLUDES = \
  -I$(aspell_includes) \
  -I$(qt_includes) \
  -I$(qt_includes)/Qt \
  -I$(qt_includes)/QtCore  \
  -I$(qt_includes)/QtXml \
  -I$(qt_includes)/QtGui \
  -I$(top_srcdir)/base  \
  -I$(top_srcdir)/base-qt

CXXFLAGS = @CXXFLAGS@

noinst_LIBRARIES = libfilesystem.a

noinst_HEADERS = \
  FileSystemFrame.h \
  FileSystemHistory.h \
  FileSystemModel.h \
  FileSystemThread.h

libfilesystem_a_SOURCES = \
	FileSystemFrame.cpp \
	FileSystemHistory.cpp \
	FileSystemModel.cpp \
	FileSystemThread.cpp

nodist_libfilesystem_a_SOURCES = \
  moc_FileSystemFrame.cpp \
  moc_FileSystemModel.cpp

clean-local:
	rm -f moc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

### document class patterns
qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
