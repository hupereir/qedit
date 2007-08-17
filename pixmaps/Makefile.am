AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libpixmaps.a
libpixmaps_a_SOURCES = qrc_pixmaps.cc

INCLUDES =  \
  -I$(qt_includes) \
  -I$(qt_includes)/Qt \
  -I$(qt_includes)/QtCore 

EXTRA_DIST = \
pixmaps.qrc \
pixmaps.pro \
editcopy.png \
editcut.png \
editpaste.png \
filenew.png \
fileopen.png \
fileprint.png \
filesave.png \
fonts.png \
icon.png \
info.png \
redo.png \
undo.png \
view_bottom.png \
view_left_right.png \
view_remove.png \
view_right.png \
view_top_bottom.png \
window_fullscreen.png

clean-local:
	rm -f qrc_*.cc

### rule to generate qt meta objects
qrc_%.cc: %.qrc
	$(qt_rcc) -name $* $^ -o $@
