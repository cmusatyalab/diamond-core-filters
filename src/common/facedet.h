/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Rahul Sukthankar
// 2003.02.05
//
// Useful routines for manipulating integral images

#ifndef FACEDET_H
#define FACEDET_H

#include "fil_tools.h"

// Recall that a feature consists of sums & differences of boxes
// defined over an integral image.  A corner is an index into an
// integral image along with a weight.
//
// [describe in these comments how a sum is computed from Corners]
//
//
typedef struct {
  int x, y;
  int weight;
} Corner;


// A Feature consists of a weighted sum of box sums &
// differences, created as a weighted sum of Corners
//
typedef struct {
  int num_corners;		// Number of corners used in this Feature
  Corner corner_list[9];	// Initialized from data.h
				// if weighted sum of BoxSums > threshold,
				// the Feature returns alpha_p, else alpha_n
  int threshold;
  double alpha_p, alpha_n;
  signed char orientation;
  signed char absv;
} Feature;


// A Level in the classifier is a list of Features along with a threshold.
//
typedef struct {
  int num_features;
  const Feature* features;	// List of Features
  double threshold;	// Region passes level if it exceeds threshold
} Level;


/* 
 * PUBLIC
 */

#ifdef __cplusplus
extern "C" {
#endif


// Integral image
typedef ii_image_t IImage;
/* typedef struct { */
/*   unsigned long** data; */
/*   int width; */
/*   int height; */
/* } IImage; */


// Call this before calling test_region()
//
void init_classifier();


//
//  Call this set the classifier to a specific scale.
void scale_feature_table(double scale);



// Runs the cascade of face detection features on the image
// (from level l1 to level l2 inclusive) and returns true
// if the cascade is positive.
int test_region(IImage* ii, int l1, int l2, int xoffset, int yoffset,
		double scale, double m);

#ifdef __cplusplus
}
#endif

#endif // FACEDET_H
