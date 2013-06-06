#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CFeature.hpp"
#include "CSURFFea.hpp"
#include "CHOGFea.hpp"
#include "tcomdef.h"

#include "cv.h"
// #include "opencv2/features2d.hpp"
#include "highgui.h"
// #include "opencv2/nonfree.hpp"

int main(int argc, char **argv)
{
    int rVal = 0;
    IplImage *pImg = cvLoadImage(argv[1]);
    IplImage *pGrayImg = cvCreateImage(cvSize(pImg->width, pImg->height),
                                       8, 1);

    TPOINT pBlockSizeList[]= {{12,12}, {16,16}, {20,20}, {24,24}, {28,28},
                              {32,32}, {36,36}, {40,40}, {12,24}, {24,12}, 
                              {16,32}, {32,16}, {16,24}, {24,16}};
    
    CFeature<double> *pCFea = TNull;
    if( (TNull == pImg) || (TNull == pGrayImg))
    {
        rVal = -1;
        goto SAM_EXIT;
    }

    
    
    pCFea = new CHOGFea<double>(NULL,(unsigned char*) (pGrayImg->imageData),
                                pGrayImg->widthStep,
                                pGrayImg->width, 
                                pGrayImg->height,
                                pBlockSizeList,
                                2,2, 396);
                                  



 SAM_EXIT:
    if(pImg) cvReleaseImage(&pImg);
    if(pGrayImg) cvReleaseImage(&pGrayImg);
    return rVal;
}
