PROJECT( DOCUMENT_CLASSES )
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

########### Qt4 configuration #########
FIND_PACKAGE( Qt4 REQUIRED )
SET( QT_USE_QTXML TRUE )

INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} )

########### includes ###############
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-qt)

IF( ASPELL_FOUND )
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-spellcheck)
ENDIF()

########### configuration files ###############
CONFIGURE_FILE( Config.h.cmake Config.h )

########### next target ###############
SET( document_classes_SOURCES
  BlockDelimiter.cpp
  BlockDelimiterDialog.cpp
  BlockDelimiterDisplay.cpp
  BlockDelimiterList.cpp
  BlockDelimiterModel.cpp
  CollapsedBlockData.cpp
  DocumentClass.cpp
  DocumentClassConfiguration.cpp
  DocumentClassDialog.cpp
  DocumentClassManager.cpp
  DocumentClassManagerDialog.cpp
  DocumentClassModel.cpp
  DocumentClassModifiedDialog.cpp
  DocumentClassPreview.cpp
  DocumentClassTextEditor.cpp
  HighlightBlockData.cpp
  HighlightBlockFlags.cpp
  HighlightPattern.cpp
  HighlightPatternDialog.cpp
  HighlightPatternList.cpp
  HighlightPatternModel.cpp
  HighlightPatternOptions.cpp
  HighlightPatternType.cpp
  HighlightStyle.cpp
  HighlightStyleList.cpp
  HighlightStyleModel.cpp
  HighlightStyleDialog.cpp
  IndentPattern.cpp
  IndentPatternDialog.cpp
  IndentPatternList.cpp
  IndentPatternModel.cpp
  IndentPatternRuleDialog.cpp
  IndentPatternRuleList.cpp
  IndentPatternRuleModel.cpp
  IndentPatternType.cpp
  ParenthesisHighlight.cpp
  TextHighlight.cpp
  TextIndent.cpp
  TextMacro.cpp
  TextMacroList.cpp
  TextMacroMenu.cpp
  TextMacroModel.cpp
  TextMacroDialog.cpp
  TextMacroRuleDialog.cpp
  TextMacroRuleList.cpp
  TextMacroRuleModel.cpp
  TextParenthesis.cpp
  TextParenthesisDialog.cpp
  TextParenthesisList.cpp
  TextParenthesisModel.cpp
)

SET( document_classes_HEADERS
  BlockDelimiterDisplay.h
  BlockDelimiterList.h
  DocumentClassModel.h
  DocumentClassDialog.h
  DocumentClassManagerDialog.h
  DocumentClassModifiedDialog.h
  DocumentClassPreview.h
  DocumentClassTextEditor.h
  HighlightStyleList.h
  HighlightPatternDialog.h
  HighlightPatternList.h
  HighlightPatternType.h
  IndentPatternList.h
  IndentPatternRuleList.h
  IndentPatternType.h
  ParenthesisHighlight.h
  TextHighlight.h
  TextIndent.h
  TextMacroList.h
  TextMacroMenu.h
  TextMacroRuleList.h
  TextParenthesisList.h
)

SET( document_classes_RESOURCES patterns.qrc)

QT4_WRAP_CPP( document_classes_HEADERS_MOC ${document_classes_HEADERS} )
QT4_ADD_RESOURCES( document_classes_RESOURCES_RCC ${document_classes_RESOURCES})

ADD_LIBRARY( document-classes STATIC
  ${document_classes_SOURCES}
  ${document_classes_HEADERS_MOC}
  ${document_classes_RESOURCES_RCC}
)
TARGET_LINK_LIBRARIES( document-classes ${QT_LIBRARIES} )
