# $Id$
project(DOCUMENT_CLASSES)

########### Qt4 configuration #########
if(USE_QT5)

  find_package(Qt5Widgets REQUIRED)

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
)

set(document_classes_RESOURCES patterns.qrc)

if(USE_QT5)

  qt5_add_resources(document_classes_RESOURCES_RCC ${document_classes_RESOURCES})

else()

  qt4_add_resources(document_classes_RESOURCES_RCC ${document_classes_RESOURCES})

endif()

add_library(document-classes STATIC
  ${document_classes_SOURCES}
  ${document_classes_RESOURCES_RCC}
)

target_link_libraries(document-classes ${QT_LIBRARIES})

if(USE_QT5)

  qt5_use_modules(document-classes Widgets Xml)

endif()
