#ifndef HISTO_H
#define HISTO_H

#include "queue.h"
#include "image_common.h"
#include "rgb.h"
#include "face.h"

// Code to support basic color histograms
// Rahul Sukthankar 2003.02.20

// FUNKY_HIST is an idea for interpolated histograms that Larry
// devised and discussed with Rahul 2003.03.24.  It is based on
// the observation that the highest bin in each dimension never
// interpolates its weight with any other bin.  Essentially, this
// is an artifact caused by using the high-order bits of a
// pixel's intensity to determine which bin it should fall in.
// FUNKY_HIST just adds a bin to the very end, guaranteeing that
// all histogram inserts will get interpolated.
//
#define FUNKY_HIST

#define HISTO_SCALE  	1.5


#define HBIT  1		// # bits to describe each dimension
// #define HBIT 1
#ifdef FUNKY_HIST
#define HBINS ((1<<HBIT)+1)	// # bins in each dimension is 2^HBIT + 1
/* static const int HBINS = ((1<<HBIT)+1); */
#else
#define HBINS (1<<HBIT)	// # bins in each dimension is 2^HBIT
/* static const int HBINS = (1<<HBIT); */
#endif // FUNKY_HIST
typedef struct {
  double data[HBINS*HBINS*HBINS];	// 3-D array in a flat format
  double weight;			// number of points in histogram
} Histo;
/* nb: there is code that relies on the above structure; see patch_spec_*_args() */

typedef struct HistoII {
  size_t   	nbytes;	      /* size of this variable sized struct */
  int 		width, height;
  int 		scalebits;
  Histo 	data[0];	/* variable size array */
} HistoII;

            
                                                                                      
/* histogram build from file name, starting at minx, miny */
typedef struct patch_t {
  Histo     histo;
  char          name[COMMON_MAX_PATH];
  uint32_t         minx, miny;
  double        threshold;  /* max distance allowed to declare similar */
  TAILQ_ENTRY(patch_t) link;
} patch_t;
                                                                                      
                                                                                      
typedef TAILQ_HEAD(patchlist_t, patch_t) patchlist_t;
                                                                                      
                                                                          
typedef struct histo_config {
    char *  name;       /* name of this search */
    int req_matches;    /* num pathes that must match */
    double  scale;      /* scale for search */
    int   xsize;      /* patch x size */
    int   ysize;      /* patch y size */
    int stride;         /* x and y strides */
    int bins;           /* number of histo bins */
    double  simularity; /* simularity metric */
    int distance_type;  /* XXX fix this */
    int num_patches;    /* num patches to match */
    patchlist_t   patchlist;
} histo_config_t;


#ifdef __cplusplus
extern "C" {
#endif

void histo_clear(Histo* h);

/* return the histogram using the ii */
void normalize_histo(Histo *hist);
/* compute a histogram from part of an image */
void histo_fill_from_subimage(Histo* h, const RGBImage* i,
			      int xstart, int ystart, int xsize, int ysize);

/* incremental computation of histogram; subimage moved from oldx,y to xstart,y. */
void histo_update_subimage(Histo* h, const RGBImage* i,
			   int oldx, int oldy,
			   int xstart, int ystart, int xsize, int ysize);

/* Returns the distance between two histogram distributions */
double histo_distance(const Histo* h1, const Histo* h2);

/* returns true if the distance between two histograms is less than d */
int histo_distance_lt(const Histo* h1, const Histo* h2, double d);

/* h1 += h2 */
void histo_accum(Histo *h1, const Histo *h2);
/* h1 -= h2 */
void histo_lessen(Histo *h1, const Histo *h2);

/*
 * compute an integral image in histo space with x,y stride of dx,
 * dy. only fills in the data field 
 */
void histo_compute_ii(const RGBImage *img, HistoII *ii, const int dx, const int dy);

/* return the histogram using the ii */
void histo_get_histo(HistoII *ii, int x, int y, int xsize, int ysize, Histo *h);


void histo_print_ii(HistoII *ii);

                                                                                      
int histo_scan_image(char *filtername, RGBImage *img,
                HistoII *ii,
                histo_config_t *fsp,
                int pthreshold, bbox_list_t *blist);
                                                                                      


#ifdef __cplusplus
}
#endif


#endif // HISTO_H