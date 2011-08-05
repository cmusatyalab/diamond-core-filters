FILTERS    += searches/ocv_face/fil_ocv

searches_ocv_face_fil_ocv_SOURCES = \
	searches/ocv_face/fil_ocv.c \
	searches/ocv_face/opencv_face_tools.c \
	searches/ocv_face/opencv_face.h \
	searches/ocv_face/opencv_face_tools.h

# Force use of the C++ linker
nodist_EXTRA_searches_ocv_face_fil_ocv_SOURCES = dummy.cxx
