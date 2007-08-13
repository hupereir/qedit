
CONFIG += qt
QT += xml network

TARGET = qedit
VERSION = Qt-4.1
DESTDIR = $$(HOME)/bin/.

INCLUDEPATH = . ../base ../base-qt ../base-server ../base-spellcheck
DEPENDPATH += . ../base ../base-qt ../base-server ../base-spellcheck

LIBS += \
  /usr/lib/libaspell.so \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  /usr/lib/libaspell.so \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

HEADERS = \
AskForSaveDialog.h \
AutoSave.h \
AutoSaveThread.h \
ConfigurationDialog.h \
Config.h \
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
TextDisplay.h \
TextHighlight.h \
TextIndent.h \
TextMacro.h \
TextParenthesis.h \
ViewHtmlDialog.h \
WindowTitle.h \
XmlDef.h

SOURCES = \
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
TextDisplay.cc \
TextHighlight.cc \
TextIndent.cc \
TextMacro.cc \
TextParenthesis.cc \
ViewHtmlDialog.cc

