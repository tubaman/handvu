# process this file with automake to create a Makefile.in

SUBDIRS = $(SUBDIR_CUBICLES) $(SUBDIR_HANDVU) \
	$(SUBDIR_HVOPENCV) $(SUBDIR_HVDC1394) $(SUBDIR_HVARTK) \
	$(SUBDIR_HVDXAPP) $(SUBDIR_HVDXFILTER) $(SUBDIR_HVCVCAM)

DIST_SUBDIRS = $(SUBDIR_CUBICLES) $(SUBDIR_HANDVU) \
	$(SUBDIR_HVOPENCV) $(SUBDIR_HVDC1394) $(SUBDIR_HVARTK) \
	$(SUBDIR_HVDXAPP) $(SUBDIR_HVDXFILTER) $(SUBDIR_HVCVCAM) \
	$(SUBDIR_HVCVCAM_DIST)

dist_pkgdata_DATA = \
config/all_extended_0_5_10_15_closed_30x20.cascade \
config/all_extended_0_5_10_15_closed_30x20.mask \
config/all_hands_combined.cascade \
config/ARtk.conductor \
config/closed.mask \
config/default.conductor \
config/FireFly4mm_calib.txt \
config/FireI.txt \
config/Lback.mask \
config/Lpalm.mask \
config/open.mask \
config/sidepoint.mask \
config/track_only.conductor \
config/victory.mask \
doc/ApplicationDocumentation.html \
doc/HandVu_Icon_green.jpg \
doc/InstallationLinux.html \
doc/video.c.patch \
doc/GestureEventFormat.pdf \
doc/HandVu_Postures.pdf \
doc/InstallationWindows.html \
doc/HandVuAPI.html \
doc/index.html \
doc/VisionConductor.pdf \
doc/rightside_up.jpg \
doc/upside_down.jpg


# All the rest of the distributed files
EXTRA_DIST = AUTHORS COPYING INSTALL README NEWS ChangeLog

# Rule to build tar-gzipped distribution package
$(PACKAGE)_$(VERSION).tar.gz: dist

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure hvconfig.h.in \
	stamp-h.in

