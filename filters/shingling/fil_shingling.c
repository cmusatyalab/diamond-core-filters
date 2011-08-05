/*
 *  Diamond Core Filters - collected filters for the Diamond platform
 *
 *  Copyright (c) 2009-2011 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>

#include "lib_filter.h"
#include "rabin.h"

struct multiset {
    size_t len;
    uint64_t elem[];
};

struct search_state {
    struct rabin_state *rpoly;
    struct multiset *example;
    struct multiset *test;
};

/* check if val is present in the set */
static int memberof(struct multiset *set, uint64_t val)
{
    unsigned int i;

    for (i = 0; i < set->len; i++)
	if (set->elem[i] == val)
	    return 1;
    return 0;
}

/* remove duplicate values from set */
static void compact_set(struct multiset *set)
{
    unsigned int len = set->len;

    for (set->len = 1; set->len < len;) {
	if (memberof(set, set->elem[set->len]))
	    set->elem[set->len] = set->elem[--len];
	else
	    set->len++;
    }
}

static double w_shingling(const void *void_data, size_t len,
			  struct rabin_state *rpoly,
			  struct multiset *set, struct multiset *tmp)
{
    unsigned int I = 0, U = set->len;
    unsigned int n = 0, s = 0, idx;
    uint64_t old_fp, new_fp;
    double r, best_r = 0.0;
    const unsigned char *data = void_data;

    if (tmp->len == 0) return 0;

    rabin_reset(rpoly);
    memset(tmp->elem, 0, tmp->len * sizeof(uint64_t));

    /* make sure we account for the 0 value(s) in tmp */
    if (memberof(set, 0)) I++;
    else		  U++;

    for (; len; data++, len--)
    {
	/* skip all whitespace */
	if (isspace(*data)) continue;

	new_fp = rabin_push(rpoly, *data);

	/* if we have not yet gotten a full shingle we shouldn't add the
	 * fingerprint to the set */
	if (++s < rpoly->windowsize) { continue; }

	/* is the new fingerprint not yet a member of tmp? */
	if (!memberof(tmp, new_fp)) {
	    if (memberof(set, new_fp)) I++;
	    else		       U++;
	}

	/* swap the new value into the set */
	idx = n++ % tmp->len;
	old_fp = tmp->elem[idx];
	tmp->elem[idx] = new_fp;

	/* was the old fingerprint the last remaining in tmp? */
	if (!memberof(tmp, old_fp)) {
	    if (memberof(set, old_fp)) I--;
	    else		       U--;
	}

	/* if we have not yet filled the search window, we shouldn't check
	 * for similarity yet */
	if (n < tmp->len) continue;

	/* remember the best match */
	r = (double)I / (double)U;
	if (r > best_r) best_r = r;
    }

    /* update length in case we were generating the initial set */
    if (set->len == 0) tmp->len = n;

    return best_r * 100;
}

static int f_init_shingling(int numarg, const char * const *args,
			    int blob_len, const void *blob,
			    const char *fname, void **data)
{
    struct search_state *state;
    struct multiset empty_set = { .len = 0 };
    unsigned int N, W, fragment_len;
    const char *fragment;

    W = atoi(args[0]);
    fragment = args[1];
    fragment_len = strlen(fragment);

    if (fragment_len <= W) {
	lf_log(LOGL_CRIT, "fil_shingling: shingle size exceeds fragment size");
	return 1;
    }

    state = malloc(sizeof(*state));

    /* initialize rabin poly */
    state->rpoly = rabin_init(FILTER_POLY, W);

    /* allocate the example set */
    N = fragment_len - W;
    state->example = malloc(sizeof(struct multiset) + N * sizeof(uint64_t));
    state->example->len = N;

    /* calculate initial set of fingerprints */
    (void)w_shingling(fragment, fragment_len, state->rpoly, &empty_set,
	    state->example);

    /* allocate and equal sized set for the test window */
    N = state->example->len;
    state->test = malloc(sizeof(struct multiset) + N * sizeof(uint64_t));
    state->test->len = N;

    /* remove unnecessary duplicates from the example set */
    compact_set(state->example);

    *data = state;
    return 0;
}

static double f_eval_shingling(lf_obj_handle_t ohandle, void *data)
{
    struct search_state *state = (struct search_state *)data;
    const void *obj;
    size_t obj_len;
    int ret;

    ret = lf_ref_attr(ohandle, "", &obj_len, &obj);
    assert(!ret);

    return w_shingling(obj, obj_len, state->rpoly, state->example, state->test);
}

#ifndef TESTING

LF_MAIN(f_init_shingling, f_eval_shingling)

#else

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void lf_log(int level, const char *fmt, ...) { return; }
int lf_ref_attr(lf_obj_handle_t o, const char *n, size_t *l, unsigned char **c) { return 0; }

int main(int argc, char **argv)
{
    unsigned char example[65536];
    size_t n;
    struct search_state *state;
    int fd;
    double R;

    n = read(0, example, sizeof(example));
    
    f_init_shingling(1, &argv[1], n, example, "", (void**)&state);

    fd = open(argv[2], O_RDONLY);
    n = read(fd, example, sizeof(example));
    close(fd);

    R = w_shingling(example, n, state->rpoly, state->example, state->test);

    printf("similarity %f\n", R);

    exit(0);
}
#endif
