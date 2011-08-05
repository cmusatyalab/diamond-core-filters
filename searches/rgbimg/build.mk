FILTERS += searches/rgbimg/fil_rgb
CODECS  += searches/rgbimg/rgb.codec

searches_rgbimg_fil_rgb_SOURCES = \
	searches/rgbimg/fil_rgb.c

searches_rgbimg_fil_rgb_LDADD = $(LDADD) -lm
