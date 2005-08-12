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

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "lib_results.h"
#include "fil_tools.h"
#include "fil_histo.h"
#include "histo.h"

/*
 ********************************************************************** */

/*
 * some prototypes 
 */

/* Accessor functions to histogram */
inline float   histo_get(const Histo * h, int ri, int gi, int bi);

/* Like histo_set() except val is added to current contents of that bin */
inline void     histo_add(Histo * h, int ri, int gi, int bi, float val);

void            histo_print(const Histo * h);

// Adds 1.0 to the bin that the pixel (r,g,b) falls into
// (no interpolation)
void            histo_simple_insert(Histo * h, int r, int g, int b);


// Just like histo_simple_insert() except that the eight cells
// closest to the pixel (r,g,b) are incremented by an appropriate
// amount -- equivalent to the weight given by bilinear
// interpolation.
//
void            histo_interpolated_insert(Histo * h, int r, int g, int b);

typedef	struct histo_lkup
{
    int		index_low;
    int		index_high;
    float	high;
    float	low;
}
histo_lkup_t;

static histo_lkup_t	red_lkup[256];
static histo_lkup_t	green_lkup[256];
static histo_lkup_t	blue_lkup[256];

static int	done_init = 0;

/*
 ********************************************************************** */


/*
 * Used inside assert() to check that we're not smashing memory 
 */
inline int
is_within_bounds(int index)
{
	if ((index >= 0) && (index < HBINS * HBINS * HBINS)) {
		return 1;
	} else {
		fprintf(stderr, "is_within_bounds(%d) failed\n", index);
		return 0;
	}
}


inline int
get_index(int ri, int gi, int bi)
{
	assert((ri >= 0) && (ri < HBINS));
	assert((gi >= 0) && (gi < HBINS));
	assert((bi >= 0) && (bi < HBINS));
	return (ri * HBINS * HBINS + gi * HBINS + bi);
}

inline float
histo_get(const Histo * h, int ri, int gi, int bi)
{
	return h->data[get_index(ri, gi, bi)];
}



inline void
histo_add(Histo * h, int ri, int gi, int bi, float val)
{
	h->data[get_index(ri, gi, bi)] += val;
	h->weight += val;
}

inline void
histo_remove(Histo * h, int ri, int gi, int bi, float val)
{
	h->data[get_index(ri, gi, bi)] -= val;
	h->weight -= val;
}

void
histo_clear(Histo * h)
{
	float         *d = h->data;
	int             i;
	for (i = 0; i < HBINS * HBINS * HBINS; i++) {
		*d++ = 0.0;
	}
	h->weight = 0.0;
}

void
histo_print(const Histo * h)
{
	int             ri,
	gi,
	bi;
	printf("weight = %f\n", h->weight);
	for (ri = 0; ri < HBINS; ri++) {
		for (gi = 0; gi < HBINS; gi++) {
			for (bi = 0; bi < HBINS; bi++) {
				float          val = histo_get(h, ri, gi, bi);
				printf("bin[%d,%d,%d] = %f\n", ri, gi, bi, val);
			}
		}
	}
}

void
histo_simple_insert(Histo * h, int r, int g, int b)
{
	assert(r >= 0 && r < 256);
	assert(g >= 0 && g < 256);
	assert(b >= 0 && b < 256);
	const int       SHIFT = 8 - HBIT;
	histo_add(h, r >> SHIFT, g >> SHIFT, b >> SHIFT, 1.0);
}

void
histo_simple_remove(Histo * h, int r, int g, int b)
{
	assert(r >= 0 && r < 256);
	assert(g >= 0 && g < 256);
	assert(b >= 0 && b < 256);
	const int       SHIFT = 8 - HBIT;
	histo_remove(h, r >> SHIFT, g >> SHIFT, b >> SHIFT, 1.0);
}


/*
 * This is Larry's hacked version of the above
 * algorithm.  The runs about 30% faster, I haven't narrowed
 * it down to which changes (loop unrolling, etc)
 * get most of the benefit.
 */

#define	HII_SHIFT 	(8 - HBIT)
#define HII_MASK 	((1<<HII_SHIFT) - 1)
#define	HII_SCALE	((float)1.0/(float)(1<<HII_SHIFT))

void
histo_interpolated_insert(Histo * h, int r, int g, int b)
{
	assert(h);
	int             rilow,
	rihigh,
	gilow,
	gihigh,
	bilow,
	bihigh;
	float          rfraclow,
	gfraclow,
	bfraclow;
	float          val;
	assert(r >= 0 && r < 256);
	assert(g >= 0 && g < 256);
	assert(b >= 0 && b < 256);

	int             ri = r >> HII_SHIFT;
	int             gi = g >> HII_SHIFT;
	int             bi = b >> HII_SHIFT;
	assert((ri >= 0) && (ri < HBINS));
	assert((gi >= 0) && (gi < HBINS));
	assert((bi >= 0) && (bi < HBINS));

	// The fractional value is given by the low-order bits
	float          rfrac = (r & HII_MASK) * HII_SCALE;
	float          gfrac = (g & HII_MASK) * HII_SCALE;
	float          bfrac = (b & HII_MASK) * HII_SCALE;
	assert((rfrac >= 0.0) && (rfrac < 1.0));
	assert((gfrac >= 0.0) && (gfrac < 1.0));
	assert((bfrac >= 0.0) && (bfrac < 1.0));


	rilow = ri * HBINS * HBINS;
	rihigh = (ri + 1) * HBINS * HBINS;
	assert(is_within_bounds(rilow));
	assert(is_within_bounds(rihigh));

	gilow = gi * HBINS;
	gihigh = (gi + 1) * HBINS;
	assert(is_within_bounds(gilow));
	assert(is_within_bounds(gihigh));

	bilow = bi;
	bihigh = (bi + 1);
	assert(is_within_bounds(bilow));
	assert(is_within_bounds(bihigh));


	rfraclow = 1.0 - rfrac;
	gfraclow = 1.0 - gfrac;
	bfraclow = 1.0 - bfrac;

	val = rfraclow * gfraclow * bfraclow;
	assert(is_within_bounds(rilow + gilow + bilow));
	h->data[rilow + gilow + bilow] += val;
	h->weight += val;

	val = rfraclow * gfraclow * bfrac;
	assert(is_within_bounds(rilow + gilow + bihigh));
	h->data[rilow + gilow + bihigh] += val;
	h->weight += val;

	val = rfraclow * gfrac * bfraclow;
	assert(is_within_bounds(rilow + gihigh + bilow));
	h->data[rilow + gihigh + bilow] += val;
	h->weight += val;

	val = rfraclow * gfrac * bfrac;
	assert(is_within_bounds(rilow + gihigh + bihigh));
	h->data[rilow + gihigh + bihigh] += val;
	h->weight += val;

	val = rfrac * gfraclow * bfraclow;
	assert(is_within_bounds(rihigh + gilow + bilow));
	h->data[rihigh + gilow + bilow] += val;
	h->weight += val;

	val = rfrac * gfraclow * bfrac;
	assert(is_within_bounds(rihigh + gilow + bihigh));
	h->data[rihigh + gilow + bihigh] += val;
	h->weight += val;

	val = rfrac * gfrac * bfraclow;
	assert(is_within_bounds(rihigh + gihigh + bilow));
	h->data[rihigh + gihigh + bilow] += val;
	h->weight += val;

	val = rfrac * gfrac * bfrac;
	assert(is_within_bounds(rihigh + gihigh + bihigh));
	h->data[rihigh + gihigh + bihigh] += val;
	h->weight += val;

}


void
build_lkuptables(histo_lkup_t * rlkup, histo_lkup_t *glkup, histo_lkup_t *blkup)
{

	float	low_frac;
	float	high_frac;
	int		i;
	int		idx;

	if (done_init) {
		return;
	}

	for (i=0; i < 256; i++) {
		idx = i >> HII_SHIFT;
		high_frac = (i & HII_MASK) * HII_SCALE;
		low_frac = 1.0 - high_frac;

		rlkup[i].high = high_frac;
		rlkup[i].low = low_frac;
		blkup[i].high = high_frac;
		blkup[i].low = low_frac;
		glkup[i].high = high_frac;
		glkup[i].low = low_frac;


		rlkup[i].index_low = idx * HBINS * HBINS;
		rlkup[i].index_high = (idx + 1) * HBINS * HBINS;

		glkup[i].index_low = idx * HBINS;
		glkup[i].index_high = (idx + 1) * HBINS;

		blkup[i].index_low = idx;
		blkup[i].index_high = (idx + 1);
	}
	done_init = 1;
}

void
lh_histo_interpolated_insert(Histo * h, int r, int g, int b)
{
	assert(h);
	int             rilow,
	rihigh,
	gilow,
	gihigh,
	bilow,
	bihigh;
	float          rfraclow, rfrac,
	gfraclow, gfrac,
	bfraclow, bfrac;
	float          val;

	// The fractional value is given by the low-order bits
	rfrac = red_lkup[r].high;
	rfraclow = red_lkup[r].low;
	bfrac = blue_lkup[b].high;
	bfraclow = blue_lkup[b].low;
	gfrac = green_lkup[g].high;
	gfraclow = green_lkup[g].low;

	rilow = red_lkup[r].index_low;
	rihigh = red_lkup[r].index_high;

	gilow = green_lkup[g].index_low;
	gihigh = green_lkup[g].index_high;

	bilow = blue_lkup[b].index_low;
	bihigh = blue_lkup[b].index_high;


	val = rfraclow * gfraclow * bfraclow;
	h->data[rilow + gilow + bilow] += val;

	val = rfraclow * gfraclow * bfrac;
	h->data[rilow + gilow + bihigh] += val;

	val = rfraclow * gfrac * bfraclow;
	h->data[rilow + gihigh + bilow] += val;

	val = rfraclow * gfrac * bfrac;
	h->data[rilow + gihigh + bihigh] += val;

	val = rfrac * gfraclow * bfraclow;
	h->data[rihigh + gilow + bilow] += val;

	val = rfrac * gfraclow * bfrac;
	h->data[rihigh + gilow + bihigh] += val;

	val = rfrac * gfrac * bfraclow;
	h->data[rihigh + gihigh + bilow] += val;

	val = rfrac * gfrac * bfrac;
	h->data[rihigh + gihigh + bihigh] += val;

	/* increase the weight (number of pixels that have been added */
	h->weight++;
}

void
histo_interpolated_remove(Histo * h, int r, int g, int b)
{
	assert(h);
	int             rilow,
	rihigh,
	gilow,
	gihigh,
	bilow,
	bihigh;
	float          rfraclow,
	gfraclow,
	bfraclow;
	float          val;
	assert(r >= 0 && r < 256);
	assert(g >= 0 && g < 256);
	assert(b >= 0 && b < 256);

	int             ri = r >> HII_SHIFT;
	int             gi = g >> HII_SHIFT;
	int             bi = b >> HII_SHIFT;
	assert((ri >= 0) && (ri < HBINS));
	assert((gi >= 0) && (gi < HBINS));
	assert((bi >= 0) && (bi < HBINS));

	// The fractional value is given by the low-order bits
	float          rfrac = (r & HII_MASK) * HII_SCALE;
	float          gfrac = (g & HII_MASK) * HII_SCALE;
	float          bfrac = (b & HII_MASK) * HII_SCALE;
	assert((rfrac >= 0.0) && (rfrac < 1.0));
	assert((gfrac >= 0.0) && (gfrac < 1.0));
	assert((bfrac >= 0.0) && (bfrac < 1.0));


	rilow = ri * HBINS * HBINS;
	rihigh = (ri + 1) * HBINS * HBINS;
	gilow = gi * HBINS;
	gihigh = (gi + 1) * HBINS;

	bilow = bi;
	bihigh = (bi + 1);

	rfraclow = 1.0 - rfrac;
	gfraclow = 1.0 - gfrac;
	bfraclow = 1.0 - bfrac;

	val = rfraclow * gfraclow * bfraclow;
	assert(is_within_bounds(rilow + gilow + bilow));
	h->data[rilow + gilow + bilow] -= val;
	h->weight -= val;

	val = rfraclow * gfraclow * bfrac;
	assert(is_within_bounds(rilow + gilow + bihigh));
	h->data[rilow + gilow + bihigh] -= val;
	h->weight -= val;

	val = rfraclow * gfrac * bfraclow;
	assert(is_within_bounds(rilow + gihigh + bilow));
	h->data[rilow + gihigh + bilow] -= val;
	h->weight -= val;

	val = rfraclow * gfrac * bfrac;
	assert(is_within_bounds(rilow + gihigh + bihigh));
	h->data[rilow + gihigh + bihigh] -= val;
	h->weight -= val;

	val = rfrac * gfraclow * bfraclow;
	assert(is_within_bounds(rihigh + gilow + bilow));
	h->data[rihigh + gilow + bilow] -= val;
	h->weight -= val;

	val = rfrac * gfraclow * bfrac;
	assert(is_within_bounds(rihigh + gilow + bihigh));
	h->data[rihigh + gilow + bihigh] -= val;
	h->weight -= val;

	val = rfrac * gfrac * bfraclow;
	assert(is_within_bounds(rihigh + gihigh + bilow));
	h->data[rihigh + gihigh + bilow] -= val;
	h->weight -= val;

	val = rfrac * gfrac * bfrac;
	assert(is_within_bounds(rihigh + gihigh + bihigh));
	h->data[rihigh + gihigh + bihigh] -= val;
	h->weight -= val;

}

void
lh_histo_interpolated_remove(Histo * h, int r, int g, int b)
{
	assert(h);
	int             rilow,
	rihigh,
	gilow,
	gihigh,
	bilow,
	bihigh;
	float          rfraclow, rfrac,
	gfraclow, gfrac,
	bfraclow, bfrac;
	float          val;

	// The fractional value is given by the low-order bits
	rfrac = red_lkup[r].high;
	rfraclow = red_lkup[r].low;
	bfrac = blue_lkup[b].high;
	bfraclow = blue_lkup[b].low;
	gfrac = green_lkup[g].high;
	gfraclow = green_lkup[g].low;

	rilow = red_lkup[r].index_low;
	rihigh = red_lkup[r].index_high;

	gilow = green_lkup[g].index_low;
	gihigh = green_lkup[g].index_high;

	bilow = blue_lkup[b].index_low;
	bihigh = blue_lkup[b].index_high;

	val = rfraclow * gfraclow * bfraclow;
	h->data[rilow + gilow + bilow] -= val;

	val = rfraclow * gfraclow * bfrac;
	h->data[rilow + gilow + bihigh] -= val;

	val = rfraclow * gfrac * bfraclow;
	h->data[rilow + gihigh + bilow] -= val;

	val = rfraclow * gfrac * bfrac;
	h->data[rilow + gihigh + bihigh] -= val;

	val = rfrac * gfraclow * bfraclow;
	h->data[rihigh + gilow + bilow] -= val;

	val = rfrac * gfraclow * bfrac;
	h->data[rihigh + gilow + bihigh] -= val;

	val = rfrac * gfrac * bfraclow;
	h->data[rihigh + gihigh + bilow] -= val;

	val = rfrac * gfrac * bfrac;
	h->data[rihigh + gihigh + bihigh] -= val;


	h->weight--;

}

inline void
histo_remove_pixel(Histo * h, const RGBPixel * p, histo_type_t type)
{
	if (type == HISTO_INTERPOLATED) {
		lh_histo_interpolated_remove(h, p->r, p->g, p->b);
	} else if (type == HISTO_SIMPLE) {
		histo_simple_remove(h, p->r, p->g, p->b);
	} else {
		assert(0);
	}
}

inline void
histo_insert_pixel(Histo * h, const RGBPixel * p, histo_type_t type)
{
	if (type == HISTO_INTERPOLATED) {
		lh_histo_interpolated_insert(h, p->r, p->g, p->b);
	} else if (type == HISTO_SIMPLE) {
		histo_simple_insert(h, p->r, p->g, p->b);
	} else {
		assert(0);
	}
}


void
histo_fill_from_subimage(Histo * h, const RGBImage * img, int xstart,
                         int ystart, int xsize, int ysize, histo_type_t type)
{
	int             i,
	j;

	if (!done_init) {
		build_lkuptables(red_lkup, green_lkup, blue_lkup);
	}
	histo_clear(h);
	for (j = 0; j < ysize; j++) {
		const RGBPixel *p = img->data + (ystart + j) * img->width + xstart;
		for (i = 0; i < xsize; i++) {
			histo_insert_pixel(h, p++, type);
		}
	}
}

void
histo_update_subimage(Histo * h, const RGBImage * img,
                      int old_xstart, int old_ystart, int new_xstart,
                      int new_ystart, int xsize, int ysize, histo_type_t type)
{
	int             xdiff = new_xstart - old_xstart;
	int             i,
	j;
	assert(old_ystart == new_ystart);

	/*
	 * Look to make sure there is enough overlap to make the differcing
	 * effective.  If not, then we go ahead and just compute the raw data.
	 */
	if (xdiff > (xsize / 2)) {
		histo_fill_from_subimage(h, img, new_xstart, new_ystart, xsize,
		                         ysize, type);
		return;
	}

	assert(xdiff <= xsize);
	/*
	 * remove the old pixels 
	 */
	for (j = 0; j < ysize; j++) {
		const RGBPixel *p =
		    img->data + (new_ystart + j) * img->width + old_xstart;
		for (i = 0; i < xdiff; i++) {
			histo_remove_pixel(h, p++, type);
		}
	}

	/*
	 * add the new pixels 
	 */
	for (j = 0; j < ysize; j++) {
		const RGBPixel *p = img->data + (new_ystart + j) * img->width +
		                    old_xstart + xsize;
		for (i = 0; i < xdiff; i++) {
			histo_insert_pixel(h, p++, type);
		}
	}
}

void
normalize_histo(Histo * h1)
{
	float          weight = h1->weight;
	int             i;
	for (i = 0; i < HBINS * HBINS * HBINS; i++) {
		h1->data[i] = h1->data[i] / weight;
	}
	h1->weight = 1.0;
}

// L1 distance over histograms
float
histo_distance(const Histo * norm_hist, const Histo * h2)
{
	int             i;
	if (norm_hist->weight != 1.0) {
		fprintf(stderr, "\nERROR ERROR!!\n");
		fprintf(stderr, "w1=%f, w2=%f\n", norm_hist->weight, h2->weight);
		return (5000.0);        /* XXX some large number */
	}
	float          weight = h2->weight;
	if (weight == 0) {
		return 0.0;
	}
	float          dist = 0.0;
	const float   *d1 = norm_hist->data;
	const float   *d2 = h2->data;
	for (i = 0; i < HBINS * HBINS * HBINS; i++) {
		dist += fabs((*d1++ * weight) - *d2++);
	}
	return 0.5 * dist / weight;
}

int
histo_distance_lt(const Histo * h1, const Histo * h2, const float d)
{
	assert(h1->weight == h2->weight);   // Not bothering to normalize
	const float    weight = h1->weight;
	if (weight == 0) {
		return 1;
	}
	float          dist = 0.0;
	const float   *d1 = h1->data;
	const float   *d2 = h2->data;
	const float    threshold = d * weight / 0.5;
	int             i;
	for (i = 0; i < HBINS * HBINS * HBINS; i++) {
		dist += fabs(*d1++ - *d2++);
		if (dist > threshold) {
			return 0;
		}
	}
	return (dist > threshold);
}

/*
 ********************************************************************** */

void
histo_accum(Histo * h1, const Histo * h2)
{
	const int       nbins = HBINS * HBINS * HBINS;
	float         *d1;
	const float   *d2;
	int             i;
	d1 = h1->data;
	d2 = h2->data;
	for (i = 0; i < nbins; i++) {
		*d1++ += *d2++;
	}
	h1->weight += h2->weight;
}


void
histo_lessen(Histo * h1, const Histo * h2)
{
	const int       nbins = HBINS * HBINS * HBINS;
	float         *d1;
	const float   *d2;
	int             i;

	d1 = h1->data;
	d2 = h2->data;
	for (i = 0; i < nbins; i++) {
		*d1++ -= *d2++;
	}
	h1->weight -= h2->weight;
}


/*
 ********************************************************************** */


#define ASSERT(exp)							\
if(!(exp)) {								\
  lf_log(LOGL_ERR, "Assertion %s failed at ", #exp);		\
  lf_log(LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);	\
  return;								\
}


void
histo_compute_ii(const RGBImage * img, HistoII * ii, const int dx,
                 const int dy, histo_type_t htype)
{
	const int       width = img->width;
	const int       height = img->height;
	int             xii;
	int             yii;
	int             x,
	y;
	Histo           hgram;

	ASSERT(dx);
	ASSERT(dy);

	ASSERT(ii->nbytes >=
	       ii->width * ii->height * sizeof(Histo) + sizeof(HistoII));
	ASSERT(ii->width >= width / dx + 1);
	ASSERT(ii->height >= height / dy + 1);

	/*
	 * zero row0, col0 
	 */
	histo_clear(&hgram);
	for (xii = 0; xii < ii->width; xii++) {
		II_PROBE(ii, xii, 0) = hgram;
	}
	for (yii = 0; yii < ii->height; yii++) {
		II_PROBE(ii, 0, yii) = hgram;
	}

	for (x = 0, xii = 1; x + dx <= width; x += dx, xii++) {
		for (y = 0, yii = 1; y + dy <= height; y += dy, yii++) {
			ASSERT(xii < ii->width);
			ASSERT(yii < ii->height);

			histo_fill_from_subimage(&hgram, img, x, y, dx, dy, htype);
			histo_accum(&hgram, &(II_PROBE(ii, xii - 1, yii)));
			histo_accum(&hgram, &(II_PROBE(ii, xii, yii - 1)));
			histo_lessen(&hgram, &(II_PROBE(ii, xii - 1, yii - 1)));
			II_PROBE(ii, xii, yii) = hgram;

		}                       /* y */
	}                           /* x */
}

HistoII *
histo_get_ii(histo_config_t *hconfig, RGBImage *img)
{
	int             ii_width, ii_height;
	int             scalebits;
	int             nbytes;
	int		gcd;
	HistoII *	ii;

	/* XXX do better on gcd for scalebits */
	if (hconfig->scale > 9000.0) {
		gcd = hconfig->stride;
	} else {
		gcd = hconfig->stride;
	}

	scalebits = log2_int(gcd);
	ii_width = (img->width >> scalebits) + 1;
	ii_height = (img->height >> scalebits) + 1;
	nbytes = ii_width * ii_height * sizeof(Histo) + sizeof(HistoII);


	ii = (HistoII *)malloc(nbytes);
	assert(ii);
	ii->nbytes = nbytes;
	ii->width = ii_width;
	ii->height = ii_height;
	ii->scalebits = scalebits;
	histo_compute_ii(img, ii, gcd, gcd, hconfig->type);

	return(ii);
}


void
histo_get_histo(HistoII * ii, int x, int y, int xsize, int ysize, Histo * h)
{
	const int       sbits = ii->scalebits;
	const int       xii = x >> sbits;
	const int       yii = y >> sbits;
	const int       dx = xsize >> sbits;
	const int       dy = ysize >> sbits;

	*h = II_PROBE(ii, xii, yii);
	histo_accum(h, &(II_PROBE(ii, xii + dx, yii + dy)));
	histo_lessen(h, &(II_PROBE(ii, xii + dx, yii)));
	histo_lessen(h, &(II_PROBE(ii, xii, yii + dy)));
}

/*
 ********************************************************************** */

void
histo_print_ii(HistoII * ii)
{
	int             x, y;
	printf("ii: %dx%d\n", ii->width, ii->height);
	for (y = 0; y < ii->height; y++) {
		for (x = 0; x < ii->width; x++) {
			printf(" %.0f", II_PROBE(ii, x, y).weight);
		}
		printf("\n");
	}
}

#ifdef XXX

/*
 * generate all the possible windows and test them 
 */
int
histo_scan_image(char *filtername, RGBImage * img, HistoII * ii,
                 histo_config_t * hconfig,
                 int num_req, bbox_list_t *blist)
{
	float          x, y;          /* XXX */
	float          d;
	dim_t           xsiz = hconfig->xsize;
	dim_t           ysiz = hconfig->ysize;
	bbox_t	 *		bbox;
	patch_t        *patch;
	int             done = 0;
	int             pass;
	float          scale;
	float          scale_factor = hconfig->scale;
	const dim_t     width = img->width;
	const dim_t     height = img->height;
	int             inspected = 0;
	Histo           h2;         /* histogram for each region tested */
	int             old_x = 0, old_y = 0;


	pass = 0;
	for (scale = 1.0; (scale * ysiz) < height; scale *= scale_factor) {
		xsiz = (dim_t) scale *hconfig->xsize;
		ysiz = (dim_t) scale *hconfig->ysize;

		for (y = 0; !done && y + ysiz <= height; y += (float)hconfig->stride) {
			if (!ii) {
				histo_fill_from_subimage(&h2, img, (int) 0, (int) y, xsiz,
				                         ysiz, hconfig->type);
				old_x = 0;
				old_y = (int) y;
			}
			for (x = 0; !done && x + xsiz <= width; x += (float)hconfig->stride) {
				inspected++;
				// histo_print_ii(ii);
				if (ii) {
					histo_get_histo(ii, (int) x, (int) y, xsiz, ysiz, &h2);
				} else {
					histo_update_subimage(&h2, img, old_x, old_y, (int) x,
					                      (int) y, xsiz, ysiz, hconfig->type);
				}
				patch = TAILQ_FIRST(&hconfig->patchlist);

				while (!done && patch) {    /* foreach patch */
					d = histo_distance(&patch->histo, &h2);

					patch = TAILQ_NEXT(patch, link);
					if (d < (1.0 - hconfig->simularity)) {  /* found match */
						bbox = (bbox_t *) malloc(sizeof(*bbox));
						bbox->min_x = x;
						bbox->min_y = y;
						bbox->max_x = x + xsiz;
						bbox->max_y = y + ysiz;
						bbox->distance = d;

						TAILQ_INSERT_TAIL(blist, bbox, link);
						pass++;
						/*
						 * use passed in pthreshold instead of
						 * fsp->pthreshold 
						 */
						if (pass >= num_req)
							done = 1;
						break;  /* no need to check other patches */
					}           /* found match */
				}               /* foreach patch */

				old_x = (int) x;
				old_y = (int) y;

			}                   /* for x */
		}                       /* for y */
	}                           /* for scale */

	return pass;
}

#else

#define	LARGE_DIST		500;
int
histo_scan_image(char *filtername, RGBImage * img, HistoII * ii,
                 histo_config_t * hconfig,
                 int num_req, bbox_list_t *blist)
{
	float          x, y;          /* XXX */
	float          d;
	dim_t           xsiz = hconfig->xsize;
	dim_t           ysiz = hconfig->ysize;
	bbox_t	 *		bbox;
	bbox_t*	  		best_box = NULL;
	patch_t        *patch;
	int             done = 0;
	int             pass;
	int				count;
	float          scale_factor = hconfig->scale;
	const dim_t     width = img->width;
	const dim_t     height = img->height;
	Histo           h2;         /* histogram for each region tested */

	assert(ii != NULL);


	/* build the lookup tables */
	/* XXX move this to init code later */
	build_lkuptables(red_lkup, green_lkup, blue_lkup);

	if (num_req < 10) {
		best_box = (bbox_t *) malloc(sizeof(*best_box) * num_req);
		for (count = 0; count < num_req; count++) {
			best_box[count].distance = LARGE_DIST;
		}
	}
	pass = 0;
	for (x=0; !done && x + hconfig->stride <= width; x += (float)hconfig->stride) {
		for (y = 0; !done && y + hconfig->stride <= height; y += (float)hconfig->stride) {
			xsiz = hconfig->xsize;
			ysiz = hconfig->ysize;
			while (((x + xsiz) < width) && ((y + ysiz) < height)) {
				histo_get_histo(ii, (int) x, (int) y, xsiz, ysiz, &h2);
				patch = TAILQ_FIRST(&hconfig->patchlist);
				while (!done && patch) {    /* foreach patch */
					d = histo_distance(&patch->histo, &h2);
					patch = TAILQ_NEXT(patch, link);

					if ((num_req < 10) && (d < (1.0 - hconfig->simularity))  &&
					    (d < best_box[num_req - 1].distance)) {  /* found match */

						bbox_t	temp_box;

						best_box[num_req - 1].min_x = x;
						best_box[num_req - 1].min_y = y;
						best_box[num_req - 1].max_x = x + xsiz;
						best_box[num_req - 1].max_y = y + ysiz;
						best_box[num_req - 1].distance = d;

						for (count = num_req - 1; count > 0; count--) {
							if (best_box[count].distance <
							    best_box[count-1].distance) {
								temp_box = best_box[count];
								best_box[count] = best_box[count - 1];
								best_box[count - 1] = temp_box;
							} else {
								break;
							}
						}
					} else if ((num_req >= 10) &&
					           (d < (1.0 - hconfig->simularity))) {  /* found match */
						bbox = (bbox_t *) malloc(sizeof(*bbox));
						bbox->min_x = x;
						bbox->min_y = y;
						bbox->max_x = x + xsiz;
						bbox->max_y = y + ysiz;
						bbox->distance = d;

						TAILQ_INSERT_TAIL(blist, bbox, link);
						pass++;
						/*
						* use passed in pthreshold instead of
						* fsp->pthreshold
						*/
						if (pass >= num_req)
							done = 1;
						break;  /* no need to check other patches */
					}
				}               /* foreach patch */

				xsiz *= scale_factor;
				ysiz *= scale_factor;
			}                   /* for x */
		}                       /* for y */
	}                           /* for scale */

	if ((num_req < 10) && (best_box[num_req - 1].distance < (1.0 - hconfig->simularity))) {
		for (count = 0; count < num_req; count++) {
			pass++;
			bbox = (bbox_t *)malloc(sizeof(*bbox));
			assert(bbox != NULL);
			bbox->min_x = best_box[count].min_x;
			bbox->min_y = best_box[count].min_y;
			bbox->max_x = best_box[count].max_x;
			bbox->max_y = best_box[count].max_y;
			bbox->distance = best_box[count].distance;
			TAILQ_INSERT_TAIL(blist, bbox, link);
		}
		//free(best_box);
	}
	if (best_box) {
		free(best_box);
	}

	return pass;
}


int
old_histo_scan_image(char *filtername, RGBImage * img, HistoII * ii,
                     histo_config_t * hconfig,
                     int num_req, bbox_list_t *blist)
{
	float          x, y;          /* XXX */
	float          d;
	dim_t           xsiz = hconfig->xsize;
	dim_t           ysiz = hconfig->ysize;
	bbox_t	 *		bbox;
	bbox_t	  		best_box;
	patch_t        *patch;
	int             done = 0;
	int             pass;
	float          scale;
	float          scale_factor = hconfig->scale;
	const dim_t     width = img->width;
	const dim_t     height = img->height;
	int             inspected = 0;
	Histo           h2;         /* histogram for each region tested */
	int             old_x = 0, old_y = 0;


	/* build the lookup tables */
	/* XXX move this to init code later */
	build_lkuptables(red_lkup, green_lkup, blue_lkup);

	best_box.distance = 500;
	pass = 0;
	for (scale = 1.0; (scale * ysiz) < height; scale *= scale_factor) {
		xsiz = (dim_t) scale *hconfig->xsize;
		ysiz = (dim_t) scale *hconfig->ysize;

		for (y = 0; !done && y + ysiz <= height; y += (float)hconfig->stride) {
			if (!ii) {
				histo_fill_from_subimage(&h2, img, (int) 0, (int) y, xsiz,
				                         ysiz, hconfig->type);
				old_x = 0;
				old_y = (int) y;
			}
			for (x = 0; !done && x + xsiz <= width; x += (float)hconfig->stride) {
				inspected++;
				// histo_print_ii(ii);
				if (ii) {
					histo_get_histo(ii, (int) x, (int) y, xsiz, ysiz, &h2);
				} else {
					histo_update_subimage(&h2, img, old_x, old_y, (int) x,
					                      (int) y, xsiz, ysiz, hconfig->type);
				}
				patch = TAILQ_FIRST(&hconfig->patchlist);

				while (!done && patch) {    /* foreach patch */
					d = histo_distance(&patch->histo, &h2);
					patch = TAILQ_NEXT(patch, link);

					if ((num_req == 1) && (d < (1.0 - hconfig->simularity))  &&
					    (d < best_box.distance)) {  /* found match */
						best_box.min_x = x;
						best_box.min_y = y;
						best_box.max_x = x + xsiz;    /* XXX scale */
						best_box.max_y = y + ysiz;
						best_box.distance = d;
					} else if ((num_req > 1) &&
					           (d < (1.0 - hconfig->simularity))) {  /* found match */
						bbox = (bbox_t *) malloc(sizeof(*bbox));
						bbox->min_x = x;
						bbox->min_y = y;
						bbox->max_x = x + xsiz;
						bbox->max_y = y + ysiz;
						bbox->distance = d;

						TAILQ_INSERT_TAIL(blist, bbox, link);
						pass++;
						/*
						* use passed in pthreshold instead of
						* fsp->pthreshold
						*/
						if (pass >= num_req)
							done = 1;
						break;  /* no need to check other patches */
					}
				}               /* foreach patch */

				old_x = (int) x;
				old_y = (int) y;

			}                   /* for x */
		}                       /* for y */
	}                           /* for scale */

	if ((num_req == 1) && (best_box.distance < (1.0 - hconfig->simularity))) {
		pass++;
		bbox = (bbox_t *)malloc(sizeof(*bbox));
		assert(bbox != NULL);
		bbox->min_x = best_box.min_x;
		bbox->min_y = best_box.min_y;
		bbox->max_x = best_box.max_x;
		bbox->max_y = best_box.max_y;
		bbox->distance = best_box.distance;
		TAILQ_INSERT_TAIL(blist, bbox, link);
	}


	return pass;
}
#endif
