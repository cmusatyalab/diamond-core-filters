FILTERS  += filters/thumbnailer/fil_thumb

filters_thumbnailer_fil_thumb_SOURCES = \
	filters/thumbnailer/fil_thumb.c

filters_thumbnailer_fil_thumb_LDADD = $(LDADD) -lm
