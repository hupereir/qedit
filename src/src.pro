TEMPLATE = app
CONFIG += qt
QT += xml network

TARGET = qedit

VERSION = 1.0
DEFINES += VERSION="\\\"$$VERSION\\\""

INCLUDEPATH = . ../base ../base-qt ../base-server ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-server ../extra-includes

LIBS += \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

RESOURCES = \
  pixmaps.qrc \
  patterns.qrc

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
 WindowTitle.h \
 XmlDef.h
 
SOURCES = \
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
 TextDisplay.cpp \
 TextHighlight.cpp \
 TextIndent.cpp \
 TextMacro.cpp \
 TextParenthesis.cpp 
