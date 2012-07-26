# $Id$

AUTOMAKE_OPTIONS = foreign
AM_CXXFLAGS = ${QT_CFLAGS}
AM_LDFLAGS = ${QT_LIBS}

EXTRA_DIST = filesystem.pro

INCLUDES = \
-I$(aspell_includes) \
-I$(top_srcdir)/base  \
-I$(top_srcdir)/base-qt

noinst_LIBRARIES = libfilesystem.a

noinst_HEADERS = \
FileInformationDialog.h \
FileRecordProperties.h \
FileSystemFrame.h \
FileSystemHistory.h \
FileSystemIcons.h \
FileSystemModel.h \
FileSystemThread.h \
RemoveFilesDialog.h \
RenameFileDialog.h


libfilesystem_a_SOURCES = \
FileInformationDialog.cpp \
FileSystemFrame.cpp \
FileSystemHistory.cpp \
FileSystemModel.cpp \
FileSystemThread.cpp \
RemoveFilesDialog.cpp \
RenameFileDialog.cpp


nodist_libfilesystem_a_SOURCES = \
moc_FileSystemFrame.cpp \
moc_FileSystemModel.cpp \
moc_RemoveFilesDialog.cpp \
moc_RenameFileDialog.cpp


clean-local:
	rm -f moc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	moc $^ -o $@

### document class patterns
qrc_%.cpp: %.qrc
	rcc -name $* $^ -o $@
