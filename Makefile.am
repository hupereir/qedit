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
 base-server \
 $(SPELLCHECK_SOURCE_DIR) \
 extra-includes \
 src

EXTRA_DIST =  \
 INSTALL \
 COPYING \
 icon.png \
 autogen.sh \
 QEdit.spec.in \
 QEdit.spec \
 QEdit.pro
