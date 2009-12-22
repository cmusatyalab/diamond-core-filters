/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _RABIN_H_
#define _RABIN_H_

#include <stdint.h>

/* 64-bit irreducible polynomial */
#define SNAPFIND_POLY 0xf00f00f00f00f001ULL

struct rabin_state {
    unsigned int windowsize;
    unsigned int shift;
    uint64_t fingerprint;
    uint64_t mod_table[256];
    uint64_t pop_table[256];
    unsigned int idx;
    unsigned char buf[];
};

struct rabin_state *rabin_init(uint64_t POLY, unsigned int W);
void rabin_free(struct rabin_state *state);
void rabin_reset(struct rabin_state *state);
uint64_t rabin_push(struct rabin_state *state, unsigned char c);

#endif /* _RABIN_H_ */
