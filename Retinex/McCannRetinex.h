#pragma once

#include "cv.h"
#include "highgui.h"
#include "stdio.h"

extern int NP, maximum;
extern IplImage *RR, *IP, *OP;

void CompareWith(int s_row, int s_col);
int frankle_mccann(IplImage *img_L, int nIterations);
