FILTERS  += searches/rgbimg/fil_rgb
SEARCHES += searches/rgbimg/rgb.search

searches_rgbimg_fil_rgb_SOURCES = \
	searches/rgbimg/fil_rgb.c

searches_rgbimg_fil_rgb_LDADD = $(LDADD) -lm
