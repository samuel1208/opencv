#pragma once 


#include "common.h"
#include "tcomdef.h"
#include "tmem.h"
#include "base-object.hpp"
/* this is the base class of the fea*/
/*
  Feature Data Type :
      0 : float 
      1 : double
      2 : int
*/

enum FEA_Method{FEA_HOG = 1};

typedef struct _TAG_FEA_PARA
{
    FEA_Method fea_method;

    //only for SURF and HOG
    int nBlockListNum;
    TPOINT *pBlockSizeList;

    //need to get, not set
    int nFeaDim; // the feature dim
    int nFeaNum; // the feature number, not the sample number
} FEA_PARA;

class CFeature : public Object
{
public :
    CFeature()
    {
        m_nFeaDim = 0;
        m_nFeaNum = 0;
        m_INTER_IMAGE_EDGE = 1;
    }
    CFeature(int nFeaDim, int nFeaNum)
    {
        m_nFeaDim = nFeaDim;
        m_nFeaNum = nFeaNum;
        m_INTER_IMAGE_EDGE = 1;
    }
    virtual ~CFeature(){}

    virtual int initial() = 0;
    virtual void unInitial() = 0;
    virtual int extractInterImg(unsigned char* pImg, int nWidthStep, 
                                int nWidth, int nHeight, int nChannel)= 0;

    // pFeaNum is a optional Para. Can set NULL,Then use "getFeaNum" get feature number
    virtual int getFea(float *pFea, TRECT rWin, int *pFeaNum) = 0;

    /*
      USAGE:
           should call after getFea
      PARA :
           rWin -- an optional parameter, SURF and HOG need
    */
    virtual int getFeaSize(TRECT rWin,int *pFeaDim, int *pFeaNum )
    {
        *pFeaDim =  m_nFeaDim;
        *pFeaNum =  m_nFeaNum;
        return SAM_OK;
    }

protected:  
    int  m_nFeaDim;
    int  m_nFeaNum;
    int  m_INTER_IMAGE_EDGE;
};



