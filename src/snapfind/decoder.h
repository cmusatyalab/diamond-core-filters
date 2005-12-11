/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _DECODER_H_
#define _DECODER_H_	1




typedef struct decoder_map {
	struct decoder_map  *	dm_next;
 	attr_decode  *		dm_decoder;
} decoder_map_t;


void decoders_init();

void decoder_register(attr_decode *decoder);
attr_decode * find_decoder(const char *name);
attr_decode * get_first_decoder(void **cookie);
attr_decode * get_next_decoder(void **cookie);
GtkWidget * get_decoder_menu(void);
attr_decode * guess_decoder(const char *name, unsigned char *data, size_t dlen);



#endif	/* ! _DECODER_H_ */

