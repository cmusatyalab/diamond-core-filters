FILTERS    += filters/img_diff/fil_img_diff

filters_img_diff_fil_img_diff_SOURCES = \
	filters/img_diff/fil_img_diff.c \
	filters/img_diff/fil_img_diff.h

filters_img_diff_fil_img_diff_LDADD = $(LDADD) -lm
