/*
 * 	Diamond (Release 1.0)
 *      A system for interactive brute-force search
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

/*
 * Program to test some wavelet stuff
 */

#define COMPILE_MAIN

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "rgb.h"

#ifdef COMPILE_MAIN
#include "image_tools.h"
#endif /* COMPILE_MAIN */

/* wvlt - wavelet library */
#include <wvlt/util.h>
#include <wvlt/lintok.h>
#include <wvlt/wvlt.h>


/*
 * Takes an input image, embeds it into a 2^k x 2^l array
 * and generates a wavelet transform.  Returns the waveletized
 * version of that image as an RGBImage (this is a 2^k x 2^l
 * object)
 */
RGBImage*
waveletize(const RGBImage* in) {
  int r, c;		/* generic row & col indices */
  int o;		/* generic offset index */
  int ydim, xdim;
  int numpix;
  double* red_frame;
  double* grn_frame;
  double* blu_frame;
  const RGBPixel* data;
  RGBImage* out = NULL;		/* allocate and return this */

  assert(in);
  fprintf(stderr, "in->height=%d in->width=%d\n", in->height, in->width);
  for (ydim = 1; ydim < in->height; ydim <<= 1) {
    assert(ydim);	/* checks overflow condition */
  }
  fprintf(stderr, "ydim=%d\n", ydim);
  for (xdim = 1; xdim < in->width; xdim <<= 1) {
    assert(xdim);	/* checks overflow condition */
  }
  fprintf(stderr, "xdim=%d\n", xdim);

  numpix = xdim * ydim;
  fprintf(stderr, "numpix=%d\n", numpix);
  red_frame = calloc(numpix, sizeof(double));
  grn_frame = calloc(numpix, sizeof(double));
  blu_frame = calloc(numpix, sizeof(double));
  assert(red_frame && grn_frame && blu_frame);

  data = in->data;
  for (r=0; r < in->height; r++) {
    for (c=0; c < in->width; c++) {

      /* offset for xxx_frame not in->data -- note different sizes */
      int offset = r * xdim + c;

      /* XXX should these pixel values be normalized b/t 0.0 and 1.0? */
      red_frame[offset] = data->r / 255.0;	
      grn_frame[offset] = data->g / 255.0;	
      blu_frame[offset] = data->b / 255.0;	
      data++;
    }
  }

#if 1
  /* WAVELET XFORM HERE */
  {
    int dim[2];
    dim[0] = ydim;
    dim[1] = xdim;
    bool isStd = TRUE;
    /* waveletfilter* wfltr = &wfltrDaubechies_4; */
    waveletfilter* wfltr = &wfltrHaar;

    /*
     * XXX
     * Seems that in-place wavelet xform is OK based on demo code
     */
    wxfrm_dand(red_frame, dim, 2, TRUE, isStd, wfltr, red_frame);
    wxfrm_dand(grn_frame, dim, 2, TRUE, isStd, wfltr, grn_frame);
    wxfrm_dand(blu_frame, dim, 2, TRUE, isStd, wfltr, blu_frame);
  }
#endif // 0

  /*
   * XXX
   *
   * RESCALE WAVELET COEEFS TO 0 -> 255
   * (this seems bogus)
   *
   */
  {
    double minr=MAXFLOAT, maxr=-MAXFLOAT;
    double ming=MAXFLOAT, maxg=-MAXFLOAT;
    double minb=MAXFLOAT, maxb=-MAXFLOAT;
    for (o=0; o<numpix; o++) {
      if (red_frame[o] < minr) { minr = red_frame[o]; }
      if (grn_frame[o] < ming) { ming = grn_frame[o]; }
      if (blu_frame[o] < minb) { minb = blu_frame[o]; }
      if (red_frame[o] > maxr) { maxr = red_frame[o]; }
      if (grn_frame[o] > maxg) { maxg = grn_frame[o]; }
      if (blu_frame[o] > maxb) { maxb = blu_frame[o]; }
    }
    fprintf(stderr, "minr=%f\tmaxr=%f\n", minr, maxr);
    fprintf(stderr, "ming=%f\tmaxb=%f\n", ming, maxg);
    fprintf(stderr, "minb=%f\tmaxb=%f\n", minb, maxb);

    /*
     * Rescale all wavelet coeffs between 0 to 1
     * (not really a good thing to do)
     * XXX
     * Warning: sloppy code:
     * the rescaling loop could fail if minX and maxX are the same...
     */
    assert(minr != maxr);
    assert(ming != maxg);
    assert(minb != maxb);
    for (o=0; o<numpix; o++) {
      red_frame[o] = (red_frame[o] - minr)/(maxr - minr);
      grn_frame[o] = (grn_frame[o] - ming)/(maxg - ming);
      blu_frame[o] = (blu_frame[o] - minb)/(maxb - minb);
    }
  }

  out = rgbimg_blank_image(xdim, ydim);
  assert(out);
  out->type = IMAGE_PPM;
  
  for (o=0; o<numpix; o++) {
    out->data[o].r = (uint8_t) (red_frame[o] * 255.0);
    out->data[o].g = (uint8_t) (grn_frame[o] * 255.0);
    out->data[o].b = (uint8_t) (blu_frame[o] * 255.0);
  }

  free(blu_frame);
  free(grn_frame);
  free(red_frame);

  return out;
}


#ifdef COMPILE_MAIN
int
main(int argc, char* argv[]) {
  RGBImage* input_image;
  RGBImage* wavelet_image;

  assert(argc == 3);
  printf("Input PPM filename (read-only) = %s\n", argv[1]);
  printf("Wavelet PPM filename = %s\n", argv[2]);

  /*
   * Read in a ppm, convert it to an RGB image,
   * send it to the wavelet-transform function,
   * get back an RGB image that contains the
   * wavelet coefficients.
   *
   * Write those out as an "image", then do the
   * inverse transform, and get back another RGB
   * image which should be the same as the original.
   */

  input_image = create_rgb_image(argv[1]);
  assert(input_image);

  wavelet_image = waveletize(input_image);
  assert(wavelet_image);

  rgb_write_image(wavelet_image, argv[2], ".");

  free(wavelet_image);
  free(input_image);

  return 0;
}
#endif /* COMPILE_MAIN */
