FILTERS += filters/rgbimg/fil_rgb

filters_rgbimg_fil_rgb_SOURCES = \
	filters/rgbimg/fil_rgb.c

filters_rgbimg_fil_rgb_LDADD = $(LDADD) -lm
