/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2005-2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_ATTR_DECODE_H_
#define	_ATTR_DECODE_H_	1

#include <gtk/gtk.h>
#include "snapfind_consts.h"



/* factory class for creating new image searches */
class attr_decode {
public:
	attr_decode(const char *name); 
	virtual ~attr_decode();
	virtual int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen) = 0;

	virtual int is_type(unsigned char *data, size_t datalen) = 0;

	const char * get_name();
	int	get_type();
	void	set_type(int type);

private:
	char * 	ad_name;
	int	ad_type;

};

/* some basic decoders that are provided */

class text_decode: public attr_decode {
public:
	text_decode():attr_decode("text") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);

private:

};

class hex_decode: public attr_decode {
public:
	hex_decode():attr_decode("hex") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);
private:
};


class int_decode: public attr_decode {
public:
	int_decode():attr_decode("integer") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);
private:
};

class time_decode: public attr_decode {
public:
	time_decode():attr_decode("time") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);
private:
};


class rgb_decode: public attr_decode {
public:
	rgb_decode():attr_decode("rgb img") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);
private:
};

class patches_decode: public attr_decode {
public:
	patches_decode():attr_decode("patches") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);
private:
};

class double_decode: public attr_decode {
public:
	double_decode():attr_decode("double") {};
	int decode(unsigned char *data, size_t datalen, 
	    char *buf, size_t buflen);
	int is_type(unsigned char *data, size_t datalen);
private:
};


#endif	/* !_ATTR_DECODE_H_ */
