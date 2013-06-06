#pragma once


#include "CFeature.hpp"

class CSURFFea : public CFeature
{
public:
    CSURFFea(THandle hMem, unsigned char* pGrayImg, 
             int nWidthStep, int nWidht, int nHeight, 
             int *pBlockSizeList, int nCellWidth, int nCellHeight, 
             int nBlockMoveStep,  int nFeaDim , int nFeaNum);
    virtual ~CSURFFea(){}
    virtual int initial();
    virtual void unInitial();
    virtual int extractInterImg();
    virtual int getFea(float **ppFea, int *pFeaDim, int *pFeaNum, TRECT rWin);

private:
    
    unsigned int *m_pInterImg;
    int m_nInterImg_width;
    int m_nInterImg_height;

    unsigned char *m_pGrayImg;
    int m_nGrayWidthStep;    
    
    int *m_pBlockSizeList;
    int m_nBlockTypeNum;
    int m_nCellWidth;
    int m_nCellHeight;
    int m_nBlockMoveStep;

};
