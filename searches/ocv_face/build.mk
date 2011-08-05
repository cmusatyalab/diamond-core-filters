FILTERS    += searches/ocv_face/fil_ocv
PREDICATES += searches/ocv_face/ocv.pred

searches_ocv_face_fil_ocv_SOURCES = \
	searches/ocv_face/fil_ocv.c \
	searches/ocv_face/opencv_face_tools.c \
	searches/ocv_face/opencv_face.h \
	searches/ocv_face/opencv_face_tools.h

# Force use of the C++ linker
nodist_EXTRA_searches_ocv_face_fil_ocv_SOURCES = dummy.cxx

OCV_FACE_BUNDLE = \
	searches/ocv_face/ocv.xml \
	searches/ocv_face/haarcascade_frontalface.xml \
	searches/ocv_face/haarcascade_fullbody.xml \
	searches/ocv_face/haarcascade_lowerbody.xml \
	searches/ocv_face/haarcascade_profileface.xml \
	searches/ocv_face/haarcascade_upperbody.xml

EXTRA_DIST += $(OCV_FACE_BUNDLE)

searches/ocv_face/ocv.pred: $(OCV_FACE_BUNDLE)
