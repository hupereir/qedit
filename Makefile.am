AUTOMAKE_OPTIONS = foreign

if WITH_ASPELL 
SPELLCHECK_SOURCE_DIR=base-spellcheck
else 
SPELLCHECK_SOURCE_DIR=
endif

SUBDIRS =  \
 patterns \
 pixmaps \
 base \
 base-qt \
 base-help \
 base-server \
 $(SPELLCHECK_SOURCE_DIR) \
 document-classes \
 extra-includes \
 src

EXTRA_DIST =  \
 INSTALL \
 COPYING \
 icon.ico \
 icon.png \
 autogen.sh \
 qedit.spec.in \
 qedit.spec \
 qedit.pro
