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

// Rahul Sukthankar
// 2003.02.05
//
// Useful routines for manipulating integral images

#ifndef FACEDET_H
#define FACEDET_H


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
 * integral image
 */
typedef struct ii_image {
        size_t    nbytes;       /* size of variable struct */
        dim_t     width, height;
        u_int32_t data[0];      /* variable size struct. width*height
                                         * elements of data follows. */
} ii_image_t;


#define II_PROBE(ii,x,y)                        \
(assert((x) >= 0), assert((y) >= 0),           \
 assert((x) < (ii)->width),                     \
 assert((y) < (ii)->height),                    \
 (ii)->data[(y) * ((ii)->width) + (x)])

typedef struct ii2_image {
        size_t    nbytes;       /* size of variable struct */
        dim_t     width, height;
        float    data[0];       /* variable size struct. width*height
                                 * elements of data follows. */
} ii2_image_t;





/*
 * PUBLIC
 */

#ifdef __cplusplus
extern "C"
{
#endif


// Integral image
typedef ii_image_t IImage;


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
