#include "McCannRetinex.h"
#include <math.h>

//int RR, IP, OP, NP, maximum;

void CompareWith(int s_row, int s_col)
{
	IP = OP;

	CvRect rect1, rect2, rect3, rect4;
	IplImage *pSubImg1, *pSubImg2, *pSubImg3, *pSubImg4;
	CvMat *pMat1, *pMat2, *pMat3, *pMat4;
	int sub_width, sub_height;

	sub_width = cvGetSize(IP).width-s_col;
	sub_height = cvGetSize(IP).height-s_row;
	if (s_row + s_col > 0)
	{	
		//get matrix 1
		pMat1 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg1 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 
		
		rect1 = cvRect(s_col, s_row, sub_width, sub_height);  
		cvGetSubRect(IP, pMat1, rect1); 
		cvGetImage(pMat1, pSubImg1);

		//get matrix 2
		pMat2 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg2 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect2 = cvRect(0, 0, sub_width, sub_height);  
		cvGetSubRect(OP, pMat2, rect2); 
		cvGetImage(pMat2, pSubImg2);

		//get matrix 3
		pMat3 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg3 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect3 = cvRect(s_col, s_row, sub_width, sub_height);  
		cvGetSubRect(RR, pMat3, rect3); 
		cvGetImage(pMat3, pSubImg3);

		//get matrix 4
		pMat4 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg4 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect4 = cvRect(0, 0, sub_width, sub_height);  
		cvGetSubRect(RR, pMat4, rect4); 
		cvGetImage(pMat4, pSubImg4);	
	
		//operation
		cvAdd(pMat2, pMat3, pMat1);
		cvSub(pMat1, pMat4, pMat4);
	
	}

	else
	{
		//get matrix 1
		pMat1 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg1 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect1 = cvRect(0, 0, sub_width, sub_height);  
		cvGetSubRect(IP, pMat1, rect1); 
		cvGetImage(pMat1, pSubImg1);

		//get matrix 2
		pMat2 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg2 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect2 = cvRect(-s_col, -s_row, sub_width, sub_height);  
		cvGetSubRect(OP, pMat2, rect2); 
		cvGetImage(pMat2, pSubImg2);

		//get matrix 3
		pMat3 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg3 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect3 = cvRect(0, 0, sub_width, sub_height);  
		cvGetSubRect(RR, pMat3, rect3); 
		cvGetImage(pMat3, pSubImg3);

		//get matrix 4
		pMat4 = cvCreateMatHeader(sub_height, sub_width, IP->depth);  
		pSubImg4 = cvCreateImageHeader(cvSize(sub_width, sub_height), IP->depth, 1); 

		rect4 = cvRect(-s_col, -s_row, sub_width, sub_height);  
		cvGetSubRect(RR, pMat4, rect4); 
		cvGetImage(pMat4, pSubImg4);	

		//operation
		cvAdd(pMat2, pMat3, pMat1);
		cvSub(pMat1, pMat4, pMat4);	
	}
	


}


int frankle_mccann(IplImage *img_L, int nIterations)
{
	int Retinex;


	return Retinex;
}