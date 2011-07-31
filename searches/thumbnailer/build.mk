FILTERS  += searches/thumbnailer/fil_thumb

searches_thumbnailer_fil_thumb_SOURCES = \
	searches/thumbnailer/fil_thumb.c

searches_thumbnailer_fil_thumb_LDADD = $(LDADD) -lm
