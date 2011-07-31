FILTERS  += searches/rgb_histogram/fil_rgb_histo
SEARCHES += searches/rgb_histogram/rgb_histo.search

searches_rgb_histogram_fil_rgb_histo_SOURCES = \
	searches/rgb_histogram/fil_rgb_histo.c \
	searches/rgb_histogram/rgb_histo.c \
	searches/rgb_histogram/rgb_histo.h

searches_rgb_histogram_fil_rgb_histo_LDADD = $(LDADD) -lm
