#pragma once 


#include "common.h"
#include "tcomdef.h"
#include "tmem.h"

/* this is the base class of the fea*/
/*
  Feature Data Type :
      0 : float 
      1 : double
      2 : int
*/

class CFeature
{
public :
    CFeature(int nFeaDim, int nFeaNum)
    {
        m_pFea = TNull;
        m_nFeaDim = nFeaDim;
        m_nFeaNum = nFeaNum;
    }
    virtual ~CFeature(){}

    virtual int initial() = 0;
    virtual void unInitial() = 0;
    virtual int extractInterImg() = 0;
    virtual int getFea(float **ppFea, int *pFeaDim, int *pFeaNum, TRECT rWin) = 0;

protected:  
    float   *m_pFea;
    int  m_nFeaDim;
    int  m_nFeaNum; //Actually this is a max Feature number 
};



