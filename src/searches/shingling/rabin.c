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

#include <stdlib.h>
#include <string.h>

#include "rabin.h"

static unsigned int degree(uint64_t p)
{
    unsigned int msb;
    for (msb = 0; p; msb++) p >>= 1;
    return msb-1;
}

/* modulo and multiplication operations in GF(2n)
 * http://en.wikipedia.org/wiki/Finite_field_arithmetic */
static uint64_t poly_mod(uint64_t x, uint64_t p)
{
    unsigned int i, d = degree(p);
    for (i = 63; i >= d; i--)
	if (x & (1ULL << i))
	    x ^= p << (i - d);
    return x;
}

static uint64_t poly_mult(uint64_t x, uint64_t y, uint64_t p)
{
    uint64_t sum = 0;
    while (x) {
	y = poly_mod(y, p);
	if (x & 1) sum ^= y;
	x >>= 1; y <<= 1;
    }
    return sum;
}

struct rabin_state *rabin_init(uint64_t POLY, unsigned int W)
{
    struct rabin_state *state;
    unsigned int i, d = degree(POLY);

    state = malloc(sizeof(struct rabin_state) + W * sizeof(unsigned char));
    state->windowsize = W;
    state->shift = d - 8;

    /* calculate which bits are flipped when we wrap around */
    for (i = 0; i < 256; i++)
	state->mod_table[i] = poly_mult(i, 1ULL<<d, POLY) ^ ((uint64_t)i<<d);

    /* clear state so we can calculate the effect of removing a value */
    rabin_reset(state);
    memset(state->pop_table, 0, 256 * sizeof(uint64_t));

    /* push a 1 followed by W-1 0's */
    rabin_push(state, 1);
    for (i = 1; i < W; i++)
	rabin_push(state, 0);

    /* calculate what bits should be flipped when bytes are popped */
    for (i = 0; i < 256; i++)
	state->pop_table[i] = poly_mult(i, state->fingerprint, POLY);

    rabin_reset(state);
    return state;
}

void rabin_free(struct rabin_state *state)
{
    free(state);
}

void rabin_reset(struct rabin_state *state)
{
    state->idx = 0;
    state->fingerprint = 0;
    memset(state->buf, 0, state->windowsize * sizeof(unsigned char));
}

uint64_t rabin_push(struct rabin_state *state, unsigned char c)
{
    unsigned char old, hi;

    /* track window with bytes added/removed */
    old = state->buf[state->idx];
    state->buf[state->idx] = c;
    if (++state->idx == state->windowsize)
	state->idx = 0;
    
    /* remove old value */
    state->fingerprint ^= state->pop_table[old];

    /* multiply by 256 and add new value */
    hi = state->fingerprint >> state->shift;
    state->fingerprint <<= 8;
    state->fingerprint |= c;

    /* modulo polynomial */
    state->fingerprint ^= state->mod_table[hi];

    return state->fingerprint;
}

