# $Id$
project(DOCUMENT_CLASSES)

########### Qt4 configuration #########
if(USE_QT5)

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Xml REQUIRED)

else()

  find_package(Qt4 REQUIRED)
  set(QT_USE_QTXML TRUE)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})

endif()

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
  DocumentClassManagerDialog.cpp
  DocumentClassModel.cpp
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

add_library(document-classes STATIC
  ${document_classes_SOURCES}
  ${document_classes_RESOURCES}
)

if(USE_QT5)

  target_link_libraries(document-classes Qt5::Widgets Qt5::Xml)

else()

  target_link_libraries(document-classes ${QT_LIBRARIES})

endif()
