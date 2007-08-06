AUTOMAKE_OPTIONS = foreign

QEditdir = $(prefix)/share/QEdit
QEdit_DATA = icon.png

if WITH_ASPELL 
SPELLCHECK_SOURCE_DIR=base-spellcheck
else 
SPELLCHECK_SOURCE_DIR=
endif

SUBDIRS =  base base-qt base-server $(SPELLCHECK_SOURCE_DIR) pixmaps src
EXTRA_DIST = INSTALL COPYING autogen.sh icon.png QEdit.spec.in QEdit.spec
