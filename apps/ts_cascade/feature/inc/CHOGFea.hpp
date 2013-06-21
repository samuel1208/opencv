#pragma once


#include "CFeature.hpp"

class CHOGFea : public CFeature
{
public:
    CHOGFea(int nWidth, int nHeight, 
            TPOINT *pBlockSizeList, int nListNum,
            int nBlockMoveStep=4,
            int nCellNumX=2, int nCellNumY=2, 
            int nHistBin=9,  int bIsPi=1);

    virtual ~CHOGFea();

public:
    virtual int initial();
    virtual void unInitial();
    virtual int extractInterImg(unsigned char* pImg, int nWidthStep, 
                                int nWidth, int nHeight, int nChannel);

    /*PARA:
           rWin : Right--Bottom open, Top--Left close
     */
    virtual int getFea(float *pFea, TRECT rWin, int *pFeaNum);
    virtual int getFeaSize(TRECT rWin, int *pFeaDim, int *pFeaNum);

private:
    int extractGradient(unsigned char* pGrayImg, int nWidthStep, 
                        int nWidth, int nHeight);
    int doScanOnInterImg();

private:
    double *m_pInterImg;
    int m_nInterWidth;
    int m_nInterHeight;
    int m_nHistBin;   
    int m_bIsPi;       /*0 : 0-360,  1: 0-180*/
    float m_fPi;
    
    TPOINT *m_pBlockSizeList;
    int     m_nListNum;
    int m_nCellNumX;
    int m_nCellNumY;
    int m_nBlockMoveStep;
};

