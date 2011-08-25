CLEANFILES += $(PREDICATES)
EXTRA_DIST += $(PREDICATES:.pred=.xml) $(OCV_FACE_BUNDLE)

PREDICATES = \
	predicates/dog_texture.pred \
	predicates/gabor_texture.pred \
	predicates/img_diff.pred \
	predicates/num_attr.pred \
	predicates/ocv_face.pred \
	predicates/perceptual_hash.pred \
	predicates/rgb_histo.pred \
	predicates/shingling.pred \
	predicates/text_attr.pred

OCV_FACE_BUNDLE = \
	predicates/ocv_face.xml \
	predicates/ocv_face/haarcascade_frontalface.xml \
	predicates/ocv_face/haarcascade_fullbody.xml \
	predicates/ocv_face/haarcascade_lowerbody.xml \
	predicates/ocv_face/haarcascade_profileface.xml \
	predicates/ocv_face/haarcascade_upperbody.xml

%.pred: %.xml
	$(BUNDLE_COMMAND)

predicates/ocv_face.pred: $(OCV_FACE_BUNDLE)
