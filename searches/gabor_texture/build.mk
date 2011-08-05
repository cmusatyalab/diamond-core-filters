FILTERS    += searches/gabor_texture/fil_gabor_texture
PREDICATES += searches/gabor_texture/gabor_texture.pred

searches_gabor_texture_fil_gabor_texture_SOURCES = \
	searches/gabor_texture/fil_gab_texture.cc \
	searches/gabor_texture/gabor.cc \
	searches/gabor_texture/gabor_filter.cc \
	searches/gabor_texture/gabor_tools.cc \
	searches/gabor_texture/gabor_filter.h \
	searches/gabor_texture/gabor.h \
	searches/gabor_texture/gabor_tools.h
