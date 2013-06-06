#pragma once


#include "CFeature.hpp"

 class CHOGFea : public CFeature
{
public:
    CHOGFea(THandle hMem, unsigned char* pGrayImg, 
            int nWidthStep, int nWidth, int nHeight, 
            TPOINT *pBlockSizeList, int nCellNumX, int nCellNumY, 
            int nFeaNum, int nBlockMoveStep=4, int nHistBin=9, 
            int bIsPi=1);

    virtual ~CHOGFea(){}
    virtual int initial();
    virtual void unInitial();
    virtual int extractInterImg();
    virtual int getFea(float **ppFea, int *pFeaDim, int *pFeaNum, TRECT rWin);

private:
    int extractGradient();
    int doScanOnInterImg();

private:
    THandle m_hMem;
    double *m_pInterImg;
    int m_nImgWidth;
    int m_nImgHeight;
    int m_nHistBin;   
    int m_bIsPi;       /*0 : 0-360,  1: 0-180*/
    float m_fPi;

    unsigned char *m_pGrayImg;
    int m_nGrayWidthStep;    
    
    TPOINT *m_pBlockSizeList;
    int m_nCellNumX;
    int m_nCellNumY;
    int m_nBlockMoveStep;
};

