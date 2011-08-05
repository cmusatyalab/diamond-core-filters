FILTERS    += filters/shingling/fil_shingling

filters_shingling_fil_shingling_SOURCES = \
	filters/shingling/fil_shingling.c \
	filters/shingling/rabin.c \
	filters/shingling/rabin.h

EXTRA_DIST += filters/shingling/genpoly.py
