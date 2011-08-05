FILTERS    += filters/ocv_face/fil_ocv

filters_ocv_face_fil_ocv_SOURCES = \
	filters/ocv_face/fil_ocv.c \
	filters/ocv_face/opencv_face_tools.c \
	filters/ocv_face/opencv_face.h \
	filters/ocv_face/opencv_face_tools.h

# Force use of the C++ linker
nodist_EXTRA_filters_ocv_face_fil_ocv_SOURCES = dummy.cxx
