#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "fil_tools.h"
#include "fil_histo.h"
#include "histo.h"

/*
 ********************************************************************** */

/*
 * some prototypes 
 */

/* Accessor functions to histogram */
inline double   histo_get(const Histo * h, int ri, int gi, int bi);

/* Like histo_set() except val is added to current contents of that bin */
inline void     histo_add(Histo * h, int ri, int gi, int bi, double val);

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

inline void     histo_interpolated_insert_pixel(Histo * h,
                                                const RGBPixel * p);


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

inline double
histo_get(const Histo * h, int ri, int gi, int bi)
{
    return h->data[get_index(ri, gi, bi)];
}



inline void
histo_add(Histo * h, int ri, int gi, int bi, double val)
{
    h->data[get_index(ri, gi, bi)] += val;
    h->weight += val;
}

void
histo_clear(Histo * h)
{
    double         *d = h->data;
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
                double          val = histo_get(h, ri, gi, bi);
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
    // printf("SHIFT = %d\n", SHIFT);
    histo_add(h, r >> SHIFT, g >> SHIFT, b >> SHIFT, 1.0);
}

void
histo_interpolated_insert(Histo * h, int r, int g, int b)
{
    assert(r >= 0 && r < 256);
    assert(g >= 0 && g < 256);
    assert(b >= 0 && b < 256);
    const int       SHIFT = 8 - HBIT;
    double          rfrac;
    double          gfrac;
    double          bfrac;
    int             i,
                    j,
                    k;

    int             ri = r >> SHIFT;
    int             gi = g >> SHIFT;
    int             bi = b >> SHIFT;

    // The fractional value is given by the low-order bits
    const int       MASK = (1 << SHIFT) - 1;
    rfrac = (r & MASK) / (double) (1 << SHIFT);
    gfrac = (g & MASK) / (double) (1 << SHIFT);
    bfrac = (b & MASK) / (double) (1 << SHIFT);


    // double sum = 0.0; // @@@@@
    for (i = 0; i <= 1; i++) {
        for (j = 0; j <= 1; j++) {
            for (k = 0; k <= 1; k++) {
                double          frac =
                    (i ? rfrac : 1.0 - rfrac) *
                    (j ? gfrac : 1.0 - gfrac) * (k ? bfrac : 1.0 - bfrac);
                // sum += frac; // @@@@@
                histo_add(h, ri + i, gi + j, bi + k, frac);
                // printf(" frac(%d,%d,%d) = %f\n", i, j, k, frac); // @@@@@
            }
        }
    }
    // printf(" sum = %f\n", sum); // @@@@@
}


/*
 * This is Larry's hacked version of the above
 * algorithm.  The runs about 30% faster, I haven't narrowed
 * it down to which changes (loop unrolling, etc)
 * get most of the benefit.
 */

#define	HII_SHIFT 	(8 - HBIT)
#define HII_MASK 	((1<<HII_SHIFT) - 1)
#define	HII_SCALE	((double)1.0/(double)(1<<HII_SHIFT))

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
    double          rfraclow,
                    gfraclow,
                    bfraclow;
    double          val;
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
    double          rfrac = (r & HII_MASK) * HII_SCALE;
    double          gfrac = (g & HII_MASK) * HII_SCALE;
    double          bfrac = (b & HII_MASK) * HII_SCALE;
    assert((rfrac >= 0.0) && (rfrac < 1.0));
    assert((gfrac >= 0.0) && (gfrac < 1.0));
    assert((bfrac >= 0.0) && (bfrac < 1.0));


    rilow = ri * HBINS * HBINS;
#ifdef FUNKY_HIST
    rihigh = (ri + 1) * HBINS * HBINS;
#else
    if (ri >= (HBINS - 1)) {
        rihigh = rilow;
    } else {
        rihigh = (ri + 1) * HBINS * HBINS;
    }
#endif                          // FUNKY_HIST
    assert(is_within_bounds(rilow));
    assert(is_within_bounds(rihigh));

    gilow = gi * HBINS;
#ifdef FUNKY_HIST
    gihigh = (gi + 1) * HBINS;
#else
    if (gi >= (HBINS - 1)) {
        gihigh = gilow;
    } else {
        gihigh = (gi + 1) * HBINS;
    }
#endif                          // FUNKY_HIST
    assert(is_within_bounds(gilow));
    assert(is_within_bounds(gihigh));

    bilow = bi;
#ifdef FUNKY_HIST
    bihigh = (bi + 1);
#else
    if (bi >= (HBINS - 1)) {
        bihigh = bilow;
    } else {
        bihigh = (bi + 1);
    }
#endif                          // FUNKY_HIST
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
lh_histo_interpolated_remove(Histo * h, int r, int g, int b)
{
    assert(h);
    int             rilow,
                    rihigh,
                    gilow,
                    gihigh,
                    bilow,
                    bihigh;
    double          rfraclow,
                    gfraclow,
                    bfraclow;
    double          val;
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
    double          rfrac = (r & HII_MASK) * HII_SCALE;
    double          gfrac = (g & HII_MASK) * HII_SCALE;
    double          bfrac = (b & HII_MASK) * HII_SCALE;
    assert((rfrac >= 0.0) && (rfrac < 1.0));
    assert((gfrac >= 0.0) && (gfrac < 1.0));
    assert((bfrac >= 0.0) && (bfrac < 1.0));


    rilow = ri * HBINS * HBINS;
#ifdef FUNKY_HIST
    rihigh = (ri + 1) * HBINS * HBINS;
#else
    if (ri >= (HBINS - 1)) {
        rihigh = rilow;
    } else {
        rihigh = (ri + 1) * HBINS * HBINS;
    }
#endif                          // FUNKY_HIST

    gilow = gi * HBINS;
#ifdef FUNKY_HIST
    gihigh = (gi + 1) * HBINS;
#else
    if (gi >= (HBINS - 1)) {
        gihigh = gilow;
    } else {
        gihigh = (gi + 1) * HBINS;
    }
#endif                          // FUNKY_HIST

    bilow = bi;
#ifdef FUNKY_HIST
    bihigh = (bi + 1);
#else
    if (bi >= (HBINS - 1)) {
        bihigh = bilow;
    } else {
        bihigh = (bi + 1);
    }
#endif                          // FUNKY_HIST


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

inline void
lh_histo_interpolated_remove_pixel(Histo * h, const RGBPixel * p)
{
    lh_histo_interpolated_remove(h, p->r, p->g, p->b);
}

inline void
lh_histo_interpolated_insert_pixel(Histo * h, const RGBPixel * p)
{
    lh_histo_interpolated_insert(h, p->r, p->g, p->b);
}
inline void
histo_interpolated_insert_pixel(Histo * h, const RGBPixel * p)
{
    histo_interpolated_insert(h, p->r, p->g, p->b);
}

void
histo_fill_from_subimage(Histo * h, const RGBImage * img,
                         int xstart, int ystart, int xsize, int ysize)
{
    int             i,
                    j;
    histo_clear(h);
    for (j = 0; j < ysize; j++) {
        const RGBPixel *p = img->data + (ystart + j) * img->width + xstart;
        for (i = 0; i < xsize; i++) {
            lh_histo_interpolated_insert_pixel(h, p++);
        }
    }
}

void
histo_update_subimage(Histo * h, const RGBImage * img,
                      int old_xstart, int old_ystart, int new_xstart,
                      int new_ystart, int xsize, int ysize)
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
                                 ysize);
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
            lh_histo_interpolated_remove_pixel(h, p++);
        }
    }

    /*
     * add the new pixels 
     */
    for (j = 0; j < ysize; j++) {
        const RGBPixel *p = img->data + (new_ystart + j) * img->width +
            old_xstart + xsize;
        for (i = 0; i < xdiff; i++) {
            lh_histo_interpolated_insert_pixel(h, p++);
        }
    }
}

void
normalize_histo(Histo * h1)
{
    double          weight = h1->weight;
    int             i;
    for (i = 0; i < HBINS * HBINS * HBINS; i++) {
        h1->data[i] = h1->data[i] / weight;
    }
    h1->weight = 1.0;
}

// L1 distance over histograms
double
histo_distance(const Histo * norm_hist, const Histo * h2)
{
    int             i;
    if (norm_hist->weight != 1.0) {
        fprintf(stderr, "\nERROR ERROR!!\n");
        fprintf(stderr, "w1=%f, w2=%f\n", norm_hist->weight, h2->weight);
        return (5000.0);        /* XXX some large number */
    }
    double          weight = h2->weight;
    if (weight == 0) {
        return 0.0;
    }
    double          dist = 0.0;
    const double   *d1 = norm_hist->data;
    const double   *d2 = h2->data;
    for (i = 0; i < HBINS * HBINS * HBINS; i++) {
        dist += fabs((*d1++ * weight) - *d2++);
    }
    return 0.5 * dist / weight;
}

int
histo_distance_lt(const Histo * h1, const Histo * h2, const double d)
{
    assert(h1->weight == h2->weight);   // Not bothering to normalize
    const double    weight = h1->weight;
    if (weight == 0) {
        return 1;
    }
    double          dist = 0.0;
    const double   *d1 = h1->data;
    const double   *d2 = h2->data;
    const double    threshold = d * weight / 0.5;
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
    double         *d1;
    const double   *d2;
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
    double         *d1;
    const double   *d2;
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
  lf_log(0, LOGL_ERR, "Assertion %s failed at ", #exp);		\
  lf_log(0, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);	\
  return;								\
}


void
histo_compute_ii(const RGBImage * img, HistoII * ii, const int dx,
                 const int dy)
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

            histo_fill_from_subimage(&hgram, img, x, y, dx, dy);
            histo_accum(&hgram, &(II_PROBE(ii, xii - 1, yii)));
            histo_accum(&hgram, &(II_PROBE(ii, xii, yii - 1)));
            histo_lessen(&hgram, &(II_PROBE(ii, xii - 1, yii - 1)));
            II_PROBE(ii, xii, yii) = hgram;

        }                       /* y */
    }                           /* x */
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

    int             x,
                    y;
    printf("ii: %dx%d\n", ii->width, ii->height);
    for (y = 0; y < ii->height; y++) {
        for (x = 0; x < ii->width; x++) {
            printf(" %.0f", II_PROBE(ii, x, y).weight);
        }
        printf("\n");
    }
}



/*
 * generate all the possible windows and test them 
 */
int
histo_scan_image(char *filtername, RGBImage * img, HistoII * ii,
                 histo_config_t * hconfig,
                 int num_req, bbox_list_t *blist)
{
    double          x,
                    y;          /* XXX */
    double          d;
    dim_t           xsiz = hconfig->xsize;
    dim_t           ysiz = hconfig->ysize;
    double          dx = (double) hconfig->stride;  /* XXX */
    double          dy = (double) hconfig->stride;
	bbox_t	 *		bbox;
    patch_t        *patch;
    int             done = 0;
    int             pass;
    double          scale;
    double          scale_factor = HISTO_SCALE;
    const dim_t     width = img->width;
    const dim_t     height = img->height;
    int             inspected = 0;
    Histo           h2;         /* histogram for each region tested */
    int             old_x = 0, old_y = 0;

    pass = 0;

    for (scale = 1.0; (scale * ysiz) < height; scale *= scale_factor) {
        xsiz = (dim_t) scale *hconfig->xsize;
        ysiz = (dim_t) scale *hconfig->ysize;
        dx = scale * (double) hconfig->stride;  /* XXX */
        dy = scale * (double) hconfig->stride;
        for (y = 0; !done && y + ysiz <= height; y += dy) {
            if (!ii) {
                histo_fill_from_subimage(&h2, img, (int) 0, (int) y, xsiz,
                                         ysiz);
                old_x = 0;
                old_y = (int) y;
            }
            for (x = 0; !done && x + xsiz <= width; x += dx) {
                inspected++;
                // histo_print_ii(ii);
                if (ii) {
                    histo_get_histo(ii, (int) x, (int) y, xsiz, ysiz, &h2);
                } else {
                    histo_update_subimage(&h2, img, old_x, old_y, (int) x,
                                          (int) y, xsiz, ysiz);
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
