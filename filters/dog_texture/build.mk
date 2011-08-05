FILTERS    += filters/dog_texture/fil_dog_texture

filters_dog_texture_fil_dog_texture_SOURCES = \
	filters/dog_texture/fil_texture.c \
	filters/dog_texture/texture_tools.c \
	filters/dog_texture/texture_tools.h

# Force use of the C++ linker
nodist_EXTRA_filters_dog_texture_fil_dog_texture_SOURCES = dummy.cxx
