FILTERS    += searches/dog_texture/fil_dog_texture

searches_dog_texture_fil_dog_texture_SOURCES = \
	searches/dog_texture/fil_texture.c \
	searches/dog_texture/texture_tools.c \
	searches/dog_texture/texture_tools.h

# Force use of the C++ linker
nodist_EXTRA_searches_dog_texture_fil_dog_texture_SOURCES = dummy.cxx
