/*
 * Copyright (c) 2006, Douglas Gray (dgray@soe.ucsc.edu, dr.de3ug@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Douglas Gray ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "retinex.h"

#include <math.h>

//#define USE_EXACT_SIGMA


#define pc(image, x, y, c) image->imageData[(image->widthStep * y) + (image->nChannels * x) + c]

#define INT_PREC 1024.0
#define INT_PREC_BITS 10

inline double int2double(int x) { return (double)x / INT_PREC; }
inline int double2int(double x) { return (int)(x * INT_PREC + 0.5); }

inline int int2smallint(int x) { return (x >> INT_PREC_BITS); }
inline int int2bigint(int x) { return (x << INT_PREC_BITS); }

//
// CreateKernel
//
// Summary:
// Creates a normalized 1 dimensional gaussian kernel.
//
// Arguments:
// sigma - the standard deviation of the gaussian kernel.
//
// Returns:
// double* - an array of values of length ((6*sigma)/2) * 2 + 1.
//
// Note:
// Caller is responsable for deleting the kernel.
//
double* CreateKernel(double sigma)
{
	int i, x, filter_size;
	double* filter;
	double sum;

	// Reject unreasonable demands
	if ( sigma > 200 ) sigma = 200;

	// get needed filter size (enforce oddness)
	filter_size = (int)floor(sigma*6) / 2;
	filter_size = filter_size * 2 + 1;

	// Allocate kernel space
	filter = new double[filter_size];

	// Calculate exponential
	sum = 0;
	for (i = 0; i < filter_size; i++) {
		x = i - (filter_size / 2);
		filter[i] = exp( -(x*x) / (2*sigma*sigma) );

		sum += filter[i];
	}

	// Normalize
	for (i = 0, x; i < filter_size; i++)
		filter[i] /= sum;

	return filter;
}

//
// CreateFastKernel
//
// Summary:
// Creates a faster gaussian kernal using integers that
// approximate floating point (leftshifted by 8 bits)
//
// Arguments:
// sigma - the standard deviation of the gaussian kernel.
//
// Returns:
// int* - an array of values of length ((6*sigma)/2) * 2 + 1.
//
// Note:
// Caller is responsable for deleting the kernel.
//

int* CreateFastKernel(double sigma)
{
	double* fp_kernel;
	int* kernel;
	int i, filter_size;
	
	// Reject unreasonable demands
	if ( sigma > 200 ) sigma = 200;

	// get needed filter size (enforce oddness)
	filter_size = (int)floor(sigma*6) / 2;
	filter_size = filter_size * 2 + 1;

	// Allocate kernel space
	kernel = new int[filter_size];

	fp_kernel = CreateKernel(sigma);

	for (i = 0; i < filter_size; i++)
		kernel[i] = double2int(fp_kernel[i]);

	delete fp_kernel;

	return kernel;
}


//
// FilterGaussian
//
// Summary:
// Performs a gaussian convolution for a value of sigma that is equal
// in both directions.
//
// Arguments:
// img - the image to be filtered in place.
// sigma - the standard deviation of the gaussian kernel to use.
//

void FilterGaussian(IplImage* img, double sigma)
{
	int i, j, k, source, filter_size;
	int* kernel;
	IplImage* temp;
	int v1, v2, v3;

	// Reject unreasonable demands
	if ( sigma > 200 ) sigma = 200;

	// get needed filter size (enforce oddness)
	filter_size = (int)floor(sigma*6) / 2;
	filter_size = filter_size * 2 + 1;

	kernel = CreateFastKernel(sigma);

	temp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
	
	// filter x axis
	for (j = 0; j < temp->height; j++)
	for (i = 0; i < temp->width; i++) {

		// inner loop has been unrolled

		v1 = v2 = v3 = 0;
		for (k = 0; k < filter_size; k++) {
			
			source = i + filter_size / 2 - k;
	
			if (source < 0) source *= -1;
			if (source > img->width - 1) source = 2*(img->width - 1) - source;

			v1 += kernel[k] * (unsigned char)pc(img, source, j, 0);
			if (img->nChannels == 1) continue;
			v2 += kernel[k] * (unsigned char)pc(img, source, j, 1);
			v3 += kernel[k] * (unsigned char)pc(img, source, j, 2);

		}

		// set value and move on
		pc(temp, i, j, 0) = (char)int2smallint(v1);
		if (img->nChannels == 1) continue;
		pc(temp, i, j, 1) = (char)int2smallint(v2);
		pc(temp, i, j, 2) = (char)int2smallint(v3);

	}
	
	// filter y axis
	for (j = 0; j < img->height; j++)
	for (i = 0; i < img->width; i++) {

		v1 = v2 = v3 = 0;
		for (k = 0; k < filter_size; k++) {
			
			source = j + filter_size / 2 - k;
	
			if (source < 0) source *= -1;
			if (source > temp->height - 1) source = 2*(temp->height - 1) - source;

			v1 += kernel[k] * (unsigned char)pc(temp, i, source, 0);
			if (img->nChannels == 1) continue;
			v2 += kernel[k] * (unsigned char)pc(temp, i, source, 1);
			v3 += kernel[k] * (unsigned char)pc(temp, i, source, 2);

		}

		// set value and move on
		pc(img, i, j, 0) = (char)int2smallint(v1);
		if (img->nChannels == 1) continue;
		pc(img, i, j, 1) = (char)int2smallint(v2);
		pc(img, i, j, 2) = (char)int2smallint(v3);

	}


	cvReleaseImage( &temp );

	delete kernel;

}

//
// FastFilter
//
// Summary:
// Performs gaussian convolution of any size sigma very fast by using
// both image pyramids and seperable filters.  Recursion is used.
//
// Arguments:
// img - an IplImage to be filtered in place.
//
void FastFilter(IplImage *img, double sigma)
{
	int filter_size;

	// Reject unreasonable demands
	if ( sigma > 200 ) sigma = 200;

	// get needed filter size (enforce oddness)
	filter_size = (int)floor(sigma*6) / 2;
	filter_size = filter_size * 2 + 1;

	// If 3 sigma is less than a pixel, why bother (ie sigma < 2/3)
	if(filter_size < 3) return;

	// Filter, or downsample and recurse
	if (filter_size < 10) {

#ifdef USE_EXACT_SIGMA
		FilterGaussian(img, sigma)
#else
		cvSmooth( img, img, CV_GAUSSIAN, filter_size, filter_size );
#endif
			
	}
	else {
		if (img->width < 2 || img->height < 2) return;

		IplImage* sub_img = cvCreateImage(cvSize(img->width / 2, img->height / 2), img->depth, img->nChannels);

		cvPyrDown( img, sub_img );

		FastFilter( sub_img, sigma / 2.0 );

		cvResize( sub_img, img, CV_INTER_LINEAR );

		cvReleaseImage( &sub_img );
	}

}

//
// Retinex
//
// Summary:
// Basic retinex restoration.  The image and a filtered image are converted
// to the log domain and subtracted.
//
// Arguments:
// img - an IplImage to be enhanced in place.
// sigma - the standard deviation of the gaussian kernal used to filter.
// gain - the factor by which to scale the image back into visable range.
// offset - an offset similar to the gain.
//
void Retinex(IplImage *img, double sigma, int gain, int offset)
{
	IplImage *A, *fA, *fB, *fC;

	// Initialize temp images
	fA = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fB = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fC = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);

	// Compute log image
	cvConvert( img, fA );
	cvLog( fA, fB );
	
	// Compute log of blured image
	A = cvCloneImage( img );
	FastFilter( A, sigma );
	cvConvert( A, fA );
	cvLog( fA, fC );

	// Compute difference
	cvSub( fB, fC, fA );

	// Restore
	cvConvertScale( fA, img, gain, offset);

	// Release temp images
	cvReleaseImage( &A );
	cvReleaseImage( &fA );
	cvReleaseImage( &fB );
	cvReleaseImage( &fC );

}

//
// MultiScaleRetinex
//
// Summary:
// Multiscale retinex restoration.  The image and a set of filtered images are
// converted to the log domain and subtracted from the original with some set
// of weights. Typicaly called with three equaly weighted scales of fine,
// medium and wide standard deviations.
//
// Arguments:
// img - an IplImage to be enhanced in place.
// sigma - the standard deviation of the gaussian kernal used to filter.
// gain - the factor by which to scale the image back into visable range.
// offset - an offset similar to the gain.
//
void MultiScaleRetinex(IplImage *img, int scales, double *weights, double *sigmas, int gain, int offset)
{
	int i;
	double weight;
	IplImage *A, *fA, *fB, *fC;

	// Initialize temp images
	fA = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fB = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fC = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);


	// Compute log image
	cvConvert( img, fA );
	cvLog( fA, fB );

	// Normalize according to given weights
	for (i = 0, weight = 0; i < scales; i++)
		weight += weights[i];

	if (weight != 1.0) cvScale( fB, fB, weight );

	// Filter at each scale
	for (i = 0; i < scales; i++) {
		A = cvCloneImage( img );
		FastFilter( A, sigmas[i] );
	
		cvConvert( A, fA );
		cvLog( fA, fC );
		cvReleaseImage( &A );

		// Compute weighted difference
		cvScale( fC, fC, weights[i] );
		cvSub( fB, fC, fB );
	}

	// Restore
	cvConvertScale( fB, img, gain, offset);

	// Release temp images
	cvReleaseImage( &fA );
	cvReleaseImage( &fB );
	cvReleaseImage( &fC );
}

//
// MultiScaleRetinexCR
//
// Summary:
// Multiscale retinex restoration with color restoration.  The image and a set of
// filtered images are converted to the log domain and subtracted from the
// original with some set of weights. Typicaly called with three equaly weighted
// scales of fine, medium and wide standard deviations. A color restoration weight
// is then applied to each color channel.
//
// Arguments:
// img - an IplImage to be enhanced in place.
// sigma - the standard deviation of the gaussian kernal used to filter.
// gain - the factor by which to scale the image back into visable range.
// offset - an offset similar to the gain.
// restoration_factor - controls the non-linearaty of the color restoration.
// color_gain - controls the color restoration gain.
//
void MultiScaleRetinexCR(IplImage *img, int scales, double *weights, double *sigmas,
					int gain, int offset, double restoration_factor, double color_gain)
{
	int i;
	double weight;
	IplImage *A, *B, *C, *fA, *fB, *fC, *fsA, *fsB, *fsC, *fsD, *fsE, *fsF;

	// Initialize temp images
	fA = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fB = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fC = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, img->nChannels);
	fsA = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, 1);
	fsB = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, 1);
	fsC = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, 1);
	fsD = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, 1);
	fsE = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, 1);
	fsF = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_32F, 1);

	// Compute log image
	cvConvert( img, fB );
	cvLog( fB, fA );

	// Normalize according to given weights
	for (i = 0, weight = 0; i < scales; i++)
		weight += weights[i];

	if (weight != 1.0) cvScale( fA, fA, weight );

	// Filter at each scale
	for (i = 0; i < scales; i++) {
		A = cvCloneImage( img );
		FastFilter( A, sigmas[i] );
	
		cvConvert( A, fB );
		cvLog( fB, fC );
		cvReleaseImage( &A );

		// Compute weighted difference
		cvScale( fC, fC, weights[i] );
		cvSub( fA, fC, fA );
	}

	// Color restoration
	if (img->nChannels > 1) {
		A = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
		B = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
		C = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
		
		// Divide image into channels, convert and store sum
		cvCvtPixToPlane( img, A, B, C, NULL );

		cvConvert( A, fsA );
		cvConvert( B, fsB );
		cvConvert( C, fsC );

		cvReleaseImage( &A );
		cvReleaseImage( &B );
		cvReleaseImage( &C );

		// Sum components
		cvAdd( fsA, fsB, fsD );
		cvAdd( fsD, fsC, fsD );

		// Normalize weights
		cvDiv( fsA, fsD, fsA, restoration_factor);
		cvDiv( fsB, fsD, fsB, restoration_factor);
		cvDiv( fsC, fsD, fsC, restoration_factor);

		cvConvertScale( fsA, fsA, 1, 1 );
		cvConvertScale( fsB, fsB, 1, 1 );
		cvConvertScale( fsC, fsC, 1, 1 );

		// Log weights
		cvLog( fsA, fsA );
		cvLog( fsB, fsB );
		cvLog( fsC, fsC );

		// Divide retinex image, weight accordingly and recombine
		cvCvtPixToPlane( fA, fsD, fsE, fsF, NULL );

		cvMul( fsD, fsA, fsD, color_gain);
		cvMul( fsE, fsB, fsE, color_gain );
		cvMul( fsF, fsC, fsF, color_gain );

		cvCvtPlaneToPix( fsD, fsE, fsF, NULL, fA );
	}

	// Restore
	cvConvertScale( fA, img, gain, offset);

	// Release temp images
	cvReleaseImage( &fA );
	cvReleaseImage( &fB );
	cvReleaseImage( &fC );
	cvReleaseImage( &fsA );
	cvReleaseImage( &fsB );
	cvReleaseImage( &fsC );
	cvReleaseImage( &fsD );
	cvReleaseImage( &fsE );
	cvReleaseImage( &fsF );
}