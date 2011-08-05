CLEANFILES += $(CODECS)
EXTRA_DIST += $(CODECS:.codec=.xml)

CODECS = \
	codecs/null.codec \
	codecs/rgb.codec

%.codec: %.xml
	$(BUNDLE_COMMAND)
