#include "CHOGFea.hpp"
#include <math.h>
/*
  Note::
  1 Don't use tri-interpolation, Because It's not fit the intergral image
  TODO ::
  1. need to add a function to check the parameters if they are correct
  2. so many parameters, need to improve.
 */

CHOGFea :: 
CHOGFea(THandle hMem, unsigned char* pGrayImg, 
        int nWidthStep, int nWidth, int nHeight, 
        TPOINT *pBlockSizeList, int nCellNumX, int nCellNumY, 
        int nFeaNum, int nBlockMoveStep, int nHistBin, int bIsPi)  :
    CFeature(nCellNumX*nCellNumY*nHistBin, nFeaNum)
{
    m_hMem = hMem;
    m_pGrayImg = pGrayImg;
    m_nGrayWidthStep = nWidthStep;    

    m_pInterImg = TNull;
    m_nImgWidth = nWidth;
    m_nImgHeight = nHeight;
    m_nHistBin = nHistBin;
    m_bIsPi = bIsPi;

    m_pBlockSizeList = pBlockSizeList;
    m_nCellNumX = nCellNumX;
    m_nCellNumY = nCellNumY;
    m_nBlockMoveStep = nBlockMoveStep;
    m_fPi = 3.1415927f; 
}

int CHOGFea :: 
initial()
{
    if(  
         (TNull == m_pGrayImg)  
         ||(m_nImgWidth <= 0)
         ||(m_nImgHeight <=0)
      )
        return SAM_ERROR;

    if(CFeature::m_nFeaDim != m_nHistBin * m_nCellNumX * m_nCellNumY)
        return SAM_ERROR;

    // alloc memory for intergral image
    int size = 0;
    size = m_nImgWidth * m_nImgHeight * m_nHistBin * sizeof(double);
    m_pInterImg = (double *)TMemAlloc(m_hMem, size);

    if(TNull == m_pInterImg)
        return SAM_ERROR;

    TMemSet(m_pInterImg, 0, size);

    // alloc memory for feature
    size = m_nFeaDim * m_nFeaNum * sizeof(float);
    m_pFea = (float*)TMemAlloc(m_hMem, size);

    if(TNull == m_pFea)
        return SAM_ERROR;

    TMemSet(m_pFea, 0, size);
    return SAM_OK;
}


void CHOGFea :: 
unInitial()
{
    if(m_pFea) 
    {
        TMemFree(m_hMem, m_pFea);
        m_pFea = TNull;
    }
    if(m_pInterImg) 
    {
        TMemFree(m_hMem, m_pInterImg);
        m_pInterImg = TNull;
    }
}

int CHOGFea :: 
extractGradient()
{
    unsigned char *pImg = m_pGrayImg;
    double *pInterImg = m_pInterImg;
    int x, y;
    int dx, dy;
    int nWidth = m_nImgWidth, nHeight = m_nImgHeight;
    int nWidthStep = m_nGrayWidthStep;
    int nInterWidthStep = nWidth * m_nHistBin;
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

    TMemSet(pInterImg, 0,  nWidth * nHeight * m_nHistBin * sizeof(double));

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
    int nWidth = m_nImgWidth, nHeight = m_nImgHeight;
    int nWidthStep = nWidth * m_nHistBin;
    double val1, val2;   
    
    if (TNull == pInterImg)
        return SAM_ERROR;

    pSum = (double *)TMemAlloc(m_hMem, nHeight * nWidthStep * sizeof(double));
    pVal = (double *)TMemAlloc(m_hMem, m_nHistBin * sizeof(double));
    if ((TNull == pSum) || (TNull == pVal))
    {
        if(pVal) TMemFree(m_hMem, pVal);
        if(pSum) TMemFree(m_hMem, pSum);
        return SAM_ERROR;
    }

    pTemp = pSum;
    //#define DEBUG_TEST
#ifdef DEBUG_TEST
    {
        FILE * file = TNull;
        char path[1024];
        for(int _t=0; _t<m_nHistBin; _t++)
        {
            TMemSet(path, 0, 1024);
            sprintf(path, "/home/samuel/tmp/sum_before_%d", _t);
            file = fopen(path, "w");
            for(int _h=0; _h<nHeight; _h++)
            {
                for(int _w=0; _w<nWidth; _w++)
                {
                    fprintf(file, "%-10.6f, ", m_pInterImg[_h*nWidthStep 
                                                    + _w*m_nHistBin + _t]);
                }
                fprintf(file, "\n");
            }
            fclose(file);            
        }
    }
#endif
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
        pInterImg += nWidthStep;

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

    TMemCpy(m_pInterImg, pSum, nHeight * nWidthStep*sizeof(double));

#ifdef DEBUG_TEST
    {
        FILE * file = TNull;
        char path[1024];
        for(int _t=0; _t<m_nHistBin; _t++)
        {
            TMemSet(path, 0, 1024);
            sprintf(path, "/home/samuel/tmp/sum_after_%d", _t);
            file = fopen(path, "w");
            for(int _h=0; _h<nHeight; _h++)
            {
                for(int _w=0; _w<nWidth; _w++)
                {
                    fprintf(file, "%-10.6f, ", m_pInterImg[_h*nWidthStep 
                                                    + _w*m_nHistBin + _t]);
                }
                fprintf(file, "\n");
            }
            fclose(file);            
        }
    }
#endif
    if(pVal) TMemFree(m_hMem, pVal);
    if(pSum) TMemFree(m_hMem, pSum);
    return SAM_OK;
}

int CHOGFea :: 
extractInterImg()
{
    if(SAM_OK != extractGradient())
        return SAM_ERROR;

    if(SAM_OK != doScanOnInterImg())
        return SAM_ERROR;

    return SAM_OK;
}

int CHOGFea :: 
getFea(float **ppFea, int *pFeaDim, int *pFeaNum, TRECT rWin)
{
    int rVal = SAM_OK;
    int nOffsetX = rWin.left, nOffsetY = rWin.top;
    int nWidth = rWin.right - rWin.left + 1;
    int nHeight = rWin.bottom - rWin.top +1;
    int nInterWidthStep = m_nImgWidth * m_nHistBin;
    
    TPOINT *pBlockSizeList = m_pBlockSizeList;
    int nFeaNum = 0;
    int nBlockTypeNum = sizeof(m_pBlockSizeList);// need test
    int i, j, w, h, x, y;
    double *pFea = TNull;
    double *pTemp = TNull;


    if( 
       (nBlockTypeNum < 1) 
       || (rWin.right > m_nImgWidth)
       || (rWin.bottom > m_nImgHeight)
       || (rWin.left<0)
       || (rWin.top<0)
       || (nWidth <= 0)
       || (nHeight <= 0)
      )
        return SAM_ERROR;

    pFea = (double*)TMemAlloc(m_hMem, m_nFeaDim*sizeof(double));
    if(TNull == pFea)
        return SAM_ERROR;
    
    for (i=0; i < nBlockTypeNum; i++)
    {
        int nBlockWidth = pBlockSizeList[i].x;
        int nBlockHeight = pBlockSizeList[i].y;
        int nMaxOffsetY = rWin.bottom - nBlockHeight;
        int nMaxOffsetX = rWin.right - nBlockWidth;
        int nCellWidth = nBlockWidth / m_nCellNumX;
        int nCellHeight = nBlockHeight / m_nCellNumY;
        for(h=nOffsetY; h<nMaxOffsetY; h+=m_nBlockMoveStep)
        {
            for(w=nOffsetX; w<nMaxOffsetX; w+=m_nBlockMoveStep)
            {
                int x1, x2, y1, y2;
                double sum = 0;
                pTemp = pFea;
                for(y=0; y<m_nCellNumY; y++)
                {
                    for(x=0; x<m_nCellNumX; x++)
                    {
                        x1 = x*nCellWidth;
                        x2 = x1 + nCellWidth;
                        y1 = y*nCellHeight;
                        y2 = y1 + nCellHeight;
                        
                        for(j=0; j<m_nHistBin; j++)
                        {
                            double v1, v2, v3, v4;
                            int index = y1*nInterWidthStep;
                            v1 = m_pInterImg[index + x1*m_nHistBin];
                            v2 = m_pInterImg[index + x2*m_nHistBin];
                            index = y2*nInterWidthStep;
                            v3 = m_pInterImg[index + x1*m_nHistBin];
                            v4 = m_pInterImg[index + x2*m_nHistBin];
                            pTemp[j] = (v4+v1) - (v2+v3);
                        }
                        pTemp += m_nHistBin;
                    }   
                }
                
                
                for (j=0; j<m_nFeaDim; j++)
                    sum += pFea[j];
                for (j=0; j<m_nFeaDim; j++)
                {
                    m_pFea[nFeaNum*m_nFeaDim +j] = (float)(pFea[j]/sum);
                }
                ++ nFeaNum ;                
            }
        }
    }
    
    if(m_nFeaNum < nFeaNum)
        rVal = SAM_ERROR;
    *ppFea = m_pFea;
    *pFeaDim = m_nFeaDim;
    *pFeaNum = nFeaNum;
    if(pFea) TMemFree(m_hMem, pFea);
    return rVal;
}
