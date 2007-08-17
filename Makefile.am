AUTOMAKE_OPTIONS = foreign

if WITH_ASPELL 
SPELLCHECK_SOURCE_DIR=base-spellcheck
else 
SPELLCHECK_SOURCE_DIR=
endif

SUBDIRS =  base base-qt base-server $(SPELLCHECK_SOURCE_DIR) patterns pixmaps src
EXTRA_DIST = INSTALL COPYING autogen.sh QEdit.spec.in QEdit.spec QEdit.pro
