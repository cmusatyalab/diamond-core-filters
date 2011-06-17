/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005 Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


#include <string.h>

#include <opencv/cv.h>

#include "rgb.h"



IplImage       *
get_gray_ipl_image(RGBImage * rgb_img)
{
	IplImage       *ipl_rgba_img;
	IplImage       *ipl_gray_img;
	int             width = rgb_img->width;
	int             height = rgb_img->height;

	ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	ipl_gray_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	/*
	 * read the data into img 
	 */

	memcpy(ipl_rgba_img->imageData, rgb_img->data, ipl_rgba_img->imageSize);

	/*
	 * create grayscale image from rgb image 
	 */
	cvCvtColor(ipl_rgba_img, ipl_gray_img, CV_RGBA2GRAY);
	cvReleaseImage(&ipl_rgba_img);
	return ipl_gray_img;
}

IplImage       *
get_rgb_ipl_image(RGBImage * rgb_img)
{
	IplImage       *ipl_rgba_img;
	IplImage       *ipl_rgb_img;
	int             width = rgb_img->width;
	int             height = rgb_img->height;

	ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	ipl_rgb_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	/*
	 * read the data into img 
	 */

	memcpy(ipl_rgba_img->imageData, rgb_img->data, ipl_rgba_img->imageSize);

	/*
	 * create grayscale image from rgb image 
	 */
	cvCvtColor(ipl_rgba_img, ipl_rgb_img, CV_RGBA2RGB);
	cvReleaseImage(&ipl_rgba_img);

	return ipl_rgb_img;
}
