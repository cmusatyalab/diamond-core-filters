FILTERS    += filters/rgb_histogram/fil_rgb_histo

filters_rgb_histogram_fil_rgb_histo_SOURCES = \
	filters/rgb_histogram/fil_rgb_histo.c \
	filters/rgb_histogram/rgb_histo.c \
	filters/rgb_histogram/rgb_histo.h

filters_rgb_histogram_fil_rgb_histo_LDADD = $(LDADD) -lm
