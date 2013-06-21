#include "CHOGFea.hpp"
#include <math.h>
#include <stdio.h>
/*
  Note::
  1 Don't use tri-interpolation, Because It's not fit the intergral image
  TODO ::
  1. need to add a function to check the parameters if they are correct
  2. so many parameters, need to improve.
 */

static int CVT_RGB2GRAY(const unsigned char *srcImg,  int srcStep,
                        unsigned char *dstImg,  int dstStep,
                        int width, int height);
static int CVT_RGB2GRAY(const unsigned char *srcImg,  int srcStep,
                        unsigned char *dstImg,  int dstStep,
                        int width, int height)
{
	const unsigned char *ptr1 = NULL;
	unsigned char *ptr2 = NULL;
    int i, j;

	if ( (NULL == srcImg) || (NULL == dstImg))
		return SAM_ERROR;


	for (i=0; i<height; i++)
	{
		ptr1 = srcImg + i*srcStep;
		ptr2 = dstImg + i*dstStep;
		for(j=0; j<width; j++)		
			ptr2[j]  = (int)(ptr1[j*3]*0.229f + ptr1[j*3+1]*0.587f + ptr1[j*3+2]*0.114f);
	}
    return SAM_OK;
}


CHOGFea :: 
CHOGFea(int nWidth, int nHeight, 
        TPOINT *pBlockSizeList, int nListNum,
        int nBlockMoveStep,
        int nCellNumX, int nCellNumY, 
        int nHistBin, int bIsPi)  :
    CFeature(nCellNumX*nCellNumY*nHistBin, -1)
{
    m_pInterImg = TNull;
    m_nInterWidth = nWidth;
    m_nInterHeight = nHeight;
    m_nHistBin = nHistBin;
    m_bIsPi = bIsPi;

    m_pBlockSizeList = pBlockSizeList;
    m_nListNum = nListNum;
    m_nCellNumX = nCellNumX;
    m_nCellNumY = nCellNumY;
    m_nBlockMoveStep = nBlockMoveStep;
    m_fPi = 3.1415927f; 
}

int CHOGFea :: 
initial()
{
    int size = 0;
    if(  (NULL == m_pBlockSizeList) 
         ||(m_nInterWidth <= 0)
         ||(m_nInterHeight <=0)
      )
        return SAM_ERROR;

    //unInitial the memory first
    unInitial();

    // alloc memory for intergral image
   
    size = (m_nInterWidth+2*m_INTER_IMAGE_EDGE) 
        *  (m_nInterHeight+2*m_INTER_IMAGE_EDGE) 
        *   m_nHistBin * sizeof(double);

    m_pInterImg = (double *)TMemAlloc(memHandle(), size);

    if(TNull == m_pInterImg)
        return SAM_ERROR;

    return SAM_OK;
}


void CHOGFea :: 
unInitial()
{
    if(m_pInterImg) 
    {
        TMemFree(memHandle(), m_pInterImg);
        m_pInterImg = TNull;
    }
}

int CHOGFea :: 
extractGradient(unsigned char* pGrayImg, int nWidthStep, 
                int nWidth, int nHeight)
{
    unsigned char *pImg = pGrayImg;
    double *pInterImg = m_pInterImg;
    int x, y;
    int dx, dy;
    int nInterWidthStep = 0;
    int index ;
    const float fPi = (1+(0==m_bIsPi))*m_fPi;
    const float fGap = fPi / m_nHistBin; 
    float grad_mag=0.0f, grad_ori=0.0f;

    ////////////////////////////////////////////////////////////
    //         simple : use 0  when the index is out of range //
    //                                 | 1 |                  //
    //                Gx=|-1 0 1|   Gy=| 0 |                  //
    //                                 |-1 |                  //
    ////////////////////////////////////////////////////////////
    
    if((TNull == pImg) || (TNull == pInterImg))
        return SAM_ERROR;

    nInterWidthStep = (nWidth+2*m_INTER_IMAGE_EDGE) * m_nHistBin;
    TMemSet(pInterImg, 0,  nInterWidthStep * (nHeight +2*m_INTER_IMAGE_EDGE) * sizeof(double));

    // skip the intergral image edge
    pInterImg = m_pInterImg 
        + m_INTER_IMAGE_EDGE*nInterWidthStep 
        + m_INTER_IMAGE_EDGE*m_nHistBin;

    for (y=0; y<nHeight; y++)
    {
        for(x=0; x<nWidth; x++)
        {
            if(0==x)  
                dx = pImg[x+1];
            else if (nWidth-1==x)
                dx = -pImg[x-1];
            else
                dx = pImg[x+1] - pImg[x-1];
            if(0==y)
                dy = -pImg[nWidth+x];
            else if(nHeight-1 == y)
                dy = pImg[-nWidth+x];
            else
                dy = -pImg[nWidth + x] + pImg[-nWidth+x];

            grad_mag = (float)sqrt((float)(dx*dx + dy*dy));
            grad_ori = (float)atan2((float)dy,(float)dx);
            if(grad_ori < 0)
                grad_ori += fPi;
            index = grad_ori / fGap;
            pInterImg[x*m_nHistBin + index] = grad_mag;            
        }
        pImg += nWidthStep;
        pInterImg += nInterWidthStep;
    }

    return SAM_OK;
}

int CHOGFea :: 
doScanOnInterImg()
{
    double *pInterImg = m_pInterImg;
    double *pTemp = TNull, *pSum = TNull;
    double *pVal = TNull;
    int x, y, n;
    int nWidth = m_nInterWidth, nHeight = m_nInterHeight;
    int nWidthStep = nWidth * m_nHistBin;
    int nInterWidthStep = (nWidth+2*m_INTER_IMAGE_EDGE) * m_nHistBin;
    double val1, val2;   
    
    if (TNull == pInterImg)
        return SAM_ERROR;

    pSum = (double *)TMemAlloc(memHandle(), nHeight * nWidthStep * sizeof(double));
    pVal = (double *)TMemAlloc(memHandle(), m_nHistBin * sizeof(double));
    if ((TNull == pSum) || (TNull == pVal))
    {
        if(pVal) TMemFree(memHandle(), pVal);
        if(pSum) TMemFree(memHandle(), pSum);
        return SAM_ERROR;
    }
    
    //#define DEBUG_TEST
#ifdef DEBUG_TEST
    {
        FILE * file = TNull;
        char path[1024];
        pInterImg = m_pInterImg 
             + m_INTER_IMAGE_EDGE*nInterWidthStep 
             + m_INTER_IMAGE_EDGE*m_nHistBin;
        for(int _t=0; _t<m_nHistBin; _t++)
        {
            TMemSet(path, 0, 1024);
            sprintf(path, "/home/samuel/tmp/sum_before_%d", _t);
            file = fopen(path, "w");
            for(int _h=0; _h<nHeight; _h++)
            {
                for(int _w=0; _w<nWidth; _w++)
                {
                    fprintf(file, "%-10.6f, ", pInterImg[_h*nInterWidthStep 
                                                         + _w*m_nHistBin + _t]);
                }
                fprintf(file, "\n");
            }
            fclose(file);            
        }
    }
#endif

    pTemp = pSum;
    pInterImg  = m_pInterImg 
        + m_INTER_IMAGE_EDGE*nInterWidthStep
        + m_INTER_IMAGE_EDGE*m_nHistBin;
    //first line
    for(n=0; n<m_nHistBin; n++)
        pTemp[n]= pInterImg[n];
    for(x=1; x<nWidth; x++)
    {
        for(n=0; n<m_nHistBin; n++)
        {
            val1 = pTemp[(x-1)*m_nHistBin +n];
            val2 = pInterImg[x*m_nHistBin +n];
            pTemp[x*m_nHistBin +n] = val1 + val2; 
        }
    }

    //rest
    for (y=1; y<nHeight; y++)
    {
        pTemp += nWidthStep;
        pInterImg += nInterWidthStep;

        for(n=0; n<m_nHistBin; n++)
        {
            pVal[n] = pInterImg[n];
            pTemp[n]= pVal[n] + pTemp[n-nWidthStep];
        }
        for(x=1; x<nWidth; x++)
        {
            for(n=0; n<m_nHistBin; n++)
            {
                val1 = pInterImg[x*m_nHistBin +n];
                pVal[n] += val1;
                val2 = pTemp[x*m_nHistBin + n-nWidthStep];
                pTemp[x*m_nHistBin + n] = pVal[n] + val2;            
            }
        }        
    }

    // copy back
    pTemp = pSum;
    pInterImg = m_pInterImg 
        + m_INTER_IMAGE_EDGE*nInterWidthStep 
        + m_INTER_IMAGE_EDGE*m_nHistBin;
    for(y=0; y<nHeight; y++)
    {
        TMemCpy(pInterImg, pTemp, nWidthStep*sizeof(double));
        pTemp += nWidthStep;
        pInterImg += nInterWidthStep;
    }

#ifdef DEBUG_TEST
    {
        FILE * file = TNull;
        char path[1024];

         pInterImg = m_pInterImg 
             + m_INTER_IMAGE_EDGE*nInterWidthStep 
             + m_INTER_IMAGE_EDGE*m_nHistBin;
        for(int _t=0; _t<m_nHistBin; _t++)
        {
            TMemSet(path, 0, 1024);
            sprintf(path, "/home/samuel/tmp/sum_after_%d", _t);
            file = fopen(path, "w");
            for(int _h=0; _h<nHeight; _h++)
            {
                for(int _w=0; _w<nWidth; _w++)
                {
                    fprintf(file, "%-10.6f, ", pInterImg[_h*nInterWidthStep 
                                                         + _w*m_nHistBin + _t]);
                }
                fprintf(file, "\n");
            }
            fclose(file);            
        }
    }
#endif
    if(pVal) TMemFree(memHandle(), pVal);
    if(pSum) TMemFree(memHandle(), pSum);
    return SAM_OK;
}

int CHOGFea :: 
extractInterImg(unsigned char* pImg, int nWidthStep, 
                int nWidth, int nHeight, int nChannel)
{
    int rVal = SAM_OK;
    unsigned char *pGrayImg = TNull;
    int nGrayWidthStep = 0;

    // convert RGB to Gray
    if(3 == nChannel)
    {
        int w,h;
        // TODO :: need to add a convertion between RGB 2 Gray  
        nGrayWidthStep = nWidth;
        pGrayImg = (unsigned char *)TMemAlloc(memHandle(), nHeight * nGrayWidthStep);
        if(TNull == pGrayImg)
            goto SAM_EXIT;

        rVal = CVT_RGB2GRAY(pImg, nWidthStep,pGrayImg, nGrayWidthStep,nWidth, nHeight);
        if(SAM_OK != rVal)
            goto SAM_EXIT;                
    }
    else if (1 == nChannel)
    {
        pGrayImg = pImg;
        nGrayWidthStep = nWidthStep;
    }

    if((m_nInterWidth!=nWidth) || (m_nInterHeight!=nHeight))
    {
        m_nInterHeight = nHeight;
        m_nInterWidth = nWidth;   
        rVal = initial();
        if(SAM_OK != rVal)
            goto SAM_EXIT;
    }
    
    rVal = extractGradient(pGrayImg, nGrayWidthStep, nWidth, nHeight);
    if(SAM_OK != rVal)
        goto SAM_EXIT;
        
    rVal = doScanOnInterImg();

 SAM_EXIT:
    if(3 == nChannel)
    {
        if(pGrayImg) TMemFree(memHandle(), pGrayImg);
    }
    return rVal;
}

int CHOGFea :: 
getFea(float *pFea, TRECT rWin, int *pFeaNum)
{
    int rVal = SAM_OK;
    int nOffsetX = rWin.left, nOffsetY = rWin.top;
    int nWidth = rWin.right - rWin.left;
    int nHeight = rWin.bottom - rWin.top;
    int nInterWidthStep = (m_nInterWidth+2*m_INTER_IMAGE_EDGE) * m_nHistBin;    
    TPOINT *pBlockSizeList = m_pBlockSizeList;
    int nFeaNum = 0;
    int nBlockTypeNum = m_nListNum;
    int i, j, w, h, x, y;
    double *pEachFea = TNull;
    double *pTemp = TNull;
    double *pInterImg = TNull;


    if( (TNull == pFea)
       ||(nBlockTypeNum < 1) 
       || (rWin.right > m_nInterWidth)
       || (rWin.bottom > m_nInterHeight)
       || (rWin.left<0)
       || (rWin.top<0)
       || (nWidth <= 0)
       || (nHeight <= 0)
      )
        return SAM_ERROR;

    pEachFea = (double*)TMemAlloc(memHandle(), m_nFeaDim*sizeof(double));
    if(TNull == pEachFea)
        return SAM_ERROR;

    pInterImg = m_pInterImg
        + m_INTER_IMAGE_EDGE*nInterWidthStep
        + m_INTER_IMAGE_EDGE*m_nHistBin;

    
    for (i=0; i < nBlockTypeNum; i++)
    {
        int nBlockWidth = pBlockSizeList[i].x;
        int nBlockHeight = pBlockSizeList[i].y;
        int nMaxOffsetY = rWin.bottom - nBlockHeight;
        int nMaxOffsetX = rWin.right - nBlockWidth;
        int nCellWidth = nBlockWidth / m_nCellNumX;
        int nCellHeight = nBlockHeight / m_nCellNumY;
        for(h=nOffsetY; h<=nMaxOffsetY; h+=m_nBlockMoveStep)
        {
            for(w=nOffsetX; w<=nMaxOffsetX; w+=m_nBlockMoveStep)
            {
                int x1, x2, y1, y2;
                double sum = 0;
                pTemp = pEachFea;
                for(y=0; y<m_nCellNumY; y++)
                {
                    for(x=0; x<m_nCellNumX; x++)
                    {
                        x1 = x*nCellWidth-1+w;
                        x2 = x1 + nCellWidth;
                        y1 = y*nCellHeight-1+h;
                        y2 = y1 + nCellHeight;
                        
                        for(j=0; j<m_nHistBin; j++)
                        {
                            double v1, v2, v3, v4;
                            int index = y1*nInterWidthStep;
                            v1 = pInterImg[index + x1*m_nHistBin+j];
                            v2 = pInterImg[index + x2*m_nHistBin+j];
                            index = y2*nInterWidthStep;
                            v3 = pInterImg[index + x1*m_nHistBin+j];
                            v4 = pInterImg[index + x2*m_nHistBin+j];
                            pTemp[j] = (v4+v1) - (v2+v3);
                        }
                        pTemp += m_nHistBin;
                    }   
                }
                
                
                for (j=0; j<m_nFeaDim; j++)
                    sum += pEachFea[j];
                if(0 == sum)
                {
                    printf("ERROR:: Divide zero in Getting features ");
                    return SAM_ERROR;
                    
                }
                for (j=0; j<m_nFeaDim; j++)
                {
                    pFea[nFeaNum*m_nFeaDim +j] = (float)(pEachFea[j]/sum);
                }
                ++ nFeaNum ;                
            }
        }
    }
    
    m_nFeaNum = nFeaNum;
    if(TNull != pFeaNum)
        *pFeaNum = nFeaNum;
    if(pEachFea) TMemFree(memHandle(), pEachFea);
    return rVal;
}

int CHOGFea::
getFeaSize(TRECT rWin, int *pFeaDim, int *pFeaNum)
{
    int i,w,h, nFeaNum = 0;
    int nBlockTypeNum = m_nListNum;
    TPOINT *pBlockSizeList = m_pBlockSizeList;
    int nOffsetX = rWin.left, nOffsetY = rWin.top;

    for (i=0; i < nBlockTypeNum; i++)
    {
        int nBlockWidth = pBlockSizeList[i].x;
        int nBlockHeight = pBlockSizeList[i].y;
        int nMaxOffsetY = rWin.bottom - nBlockHeight;
        int nMaxOffsetX = rWin.right - nBlockWidth;
        int nCellWidth = nBlockWidth / m_nCellNumX;
        int nCellHeight = nBlockHeight / m_nCellNumY;
        
        for(h=nOffsetY; h<=nMaxOffsetY; h+=m_nBlockMoveStep)
        {
            for(w=nOffsetX; w<=nMaxOffsetX; w+=m_nBlockMoveStep)
            {
                ++ nFeaNum ;                
            }
        }
    }    
    m_nFeaNum = nFeaNum;
    *pFeaDim =  m_nFeaDim;
    *pFeaNum =  m_nFeaNum;
    return SAM_OK;
}

CHOGFea::
~CHOGFea()
{
    unInitial();
}
