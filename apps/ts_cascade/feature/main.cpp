#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CFeature.hpp"
#include "CSURFFea.hpp"
#include "CHOGFea.hpp"
#include "tcomdef.h"
#include <typeinfo>
#include "cv.h"
// #include "opencv2/features2d.hpp"
#include "highgui.h"
// #include "opencv2/nonfree.hpp"

int main(int argc, char **argv)
{
    int rVal = 0;
    IplImage *pImg = TNull;
    IplImage *pGrayImg = TNull;

    TPOINT pBlockSizeList[]= {{12,12}, {16,16}, {20,20}, {24,24}, {28,28},
                              {32,32}, {36,36}, {40,40}, {12,24}, {24,12}, 
                              {16,32}, {32,16}, {16,24}, {24,16}};
    
    CFeature  *pCFea = TNull;
    float *pFea = TNull;
    int nFeaDim, nFeaNum;
    TRECT rWin={0,0,40,40};

    if(argc < 1)
    {
        rVal = -1;
        goto SAM_EXIT;
    }

    pImg = cvLoadImage(argv[1]);
    if (TNull == pImg) 
    {
        rVal = -1;
        goto SAM_EXIT;
    }

    pGrayImg = cvCreateImage(cvSize(pImg->width, pImg->height),8,1);
    if (TNull == pGrayImg)
    {
        rVal = -1;
        goto SAM_EXIT;
    }

    cvCvtColor(pImg, pGrayImg, CV_RGB2GRAY);
    
    pCFea = new CHOGFea(NULL, (unsigned char *)pGrayImg->imageData,
                                pGrayImg->widthStep,
                                pGrayImg->width, 
                                pGrayImg->height,
                                pBlockSizeList,
                                2,2, 500);
    rVal = pCFea->initial();
    rVal = pCFea->extractInterImg();
    rVal = pCFea->getFea(&pFea, &nFeaDim, &nFeaNum, rWin);
    pCFea->unInitial();      



 SAM_EXIT:
    if(pImg) cvReleaseImage(&pImg);
    if(pGrayImg) cvReleaseImage(&pGrayImg);
    return rVal;
}
