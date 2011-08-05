FILTERS =
CODECS =
PREDICATES =

include searches/dog_texture/build.mk
include searches/gabor_texture/build.mk
include searches/img_diff/build.mk
include searches/null/build.mk
include searches/num_attr/build.mk
include searches/ocv_face/build.mk
include searches/rgb_histogram/build.mk
include searches/rgbimg/build.mk
include searches/shingling/build.mk
include searches/text_attr/build.mk
include searches/thumbnailer/build.mk

CLEANFILES  = $(CODECS) $(PREDICATES)
EXTRA_DIST += $(PREDICATES:.codec=.xml) $(PREDICATES:.pred=.xml)

AM_V_BUNDLE   = $(AM_V_BUNDLE_$(V))
AM_V_BUNDLE_  = $(AM_V_BUNDLE_$(AM_DEFAULT_VERBOSITY))
AM_V_BUNDLE_0 = @echo "  BUNDLE" $@;
%.pred: %.xml
	$(AM_V_BUNDLE) $(DIAMOND_BUNDLE_PREDICATE) -o $@ $^
%.codec: %.xml
	$(AM_V_BUNDLE) $(DIAMOND_BUNDLE_PREDICATE) -o $@ $^
