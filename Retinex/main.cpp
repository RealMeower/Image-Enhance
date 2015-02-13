#include "retinex.h"

void main()
{
	/******************************MSR***********************************/
	IplImage *img;
	IplImage *dstimg;
	IplImage *imgR, *imgG, *imgB;
	double weights[] = {0.33, 0.34, 0.33};
	double sigmas[] = {15, 80, 250};

	img = cvLoadImage("B.jpg", 1);
	cvNamedWindow("A", CV_WINDOW_AUTOSIZE);
	cvShowImage("A",img);
	imgR = cvCreateImage(cvGetSize(img), img->depth, 1);
	imgG = cvCreateImage(cvGetSize(img), img->depth, 1);
	imgB = cvCreateImage(cvGetSize(img), img->depth, 1);

	cvSplit(img, imgB, imgG, imgR, NULL);
	MultiScaleRetinex(imgR, 3, weights, sigmas, 128, 128);
	MultiScaleRetinex(imgG, 3, weights, sigmas, 128, 128);
	MultiScaleRetinex(imgB, 3, weights, sigmas, 128, 128);

	dstimg = cvCreateImage(cvGetSize(img), img->depth, 3);
	cvMerge(imgB, imgG, imgR, NULL, dstimg);
	cvNamedWindow("B", CV_WINDOW_AUTOSIZE);
	cvShowImage("B",dstimg);
	cvWaitKey(0);

	cvSaveImage("C.jpg", dstimg);

}