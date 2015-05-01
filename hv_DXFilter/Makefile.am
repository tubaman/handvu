
#bin_PROGRAMS = HandVuFilter.ax

HandVuFilter_SOURCES = \
DllTester.cpp HandVuFilterProp.cpp \
HandVuFilter.cpp HandVuFilterProp.rc MaintenanceApp.cpp \
HandVuFilter.def IconGroup.cpp

noinst_HEADERS = HandVuFilter.h IconGroup.h IHandVuFilter.h \
HandVuFilterGUIDs.h MaintenanceApp.h \
resource.h HandVuFilterProp.h ImageOverlay.h StdAfx.h


EXTRA_DIST = HandVuFilter.vcproj DllTester.vcproj $(HandVuFilter_SOURCES) \
	FrameDataLib.h FrameDataLib.lib FrameDataLib.dll
