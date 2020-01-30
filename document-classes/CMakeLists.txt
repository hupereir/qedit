# $Id$
project(DOCUMENT_CLASSES)

########### Qt configuration #########
find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

########### includes ###############
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/base-qt)

if(ASPELL_FOUND)
  include_directories(${ASPELL_INCLUDE_DIR})
  include_directories(${CMAKE_SOURCE_DIR}/base-spellcheck)
endif()

########### next target ###############
set(document_classes_SOURCES
  BlockDelimiter.cpp
  BlockDelimiterDisplay.cpp
  CollapsedBlockData.cpp
  DocumentClass.cpp
  DocumentClassManager.cpp
  HighlightBlockData.cpp
  HighlightPattern.cpp
  HighlightStyle.cpp
  IndentPattern.cpp
  ParenthesisHighlight.cpp
  PatternLocation.cpp
  PatternLocationSet.cpp
  TextBlockDelimiter.cpp
  TextHighlight.cpp
  TextIndent.cpp
  TextMacro.cpp
  TextMacroMenu.cpp
  TextParenthesis.cpp
  XmlString.cpp
)

set(document_classes_RESOURCES patterns.qrc)

add_library(document-classes STATIC ${document_classes_SOURCES} ${document_classes_RESOURCES})
target_link_libraries(document-classes Qt5::Widgets Qt5::Xml)
