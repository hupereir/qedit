# $Id$

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
  BlockDelimiterDisplay.cpp
  CollapsedBlockData.cpp
  DocumentClass.cpp
  DocumentClassManager.cpp
  DocumentClassModel.cpp
  HighlightBlockData.cpp
  HighlightBlockFlags.cpp
  HighlightPattern.cpp
  HighlightStyle.cpp
  IndentPattern.cpp
  ParenthesisHighlight.cpp
  TextHighlight.cpp
  TextIndent.cpp
  TextMacro.cpp
  TextMacroMenu.cpp
  TextParenthesis.cpp
)

SET( document_classes_HEADERS
  BlockDelimiterDisplay.h
  DocumentClassModel.h
  ParenthesisHighlight.h
  TextHighlight.h
  TextIndent.h
  TextMacroMenu.h
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
