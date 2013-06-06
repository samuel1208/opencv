#pragma once

#include "tcomdef.h"
#include "tmem.h"

class CWeakLearner
{
public :
    CWeakLearner(){}
    virtual ~CWeakLearner(){}

    virtual int initial() = 0;
    virtual void unInitial() = 0;

    virtual int solve(float *pFeaPos, int nFeaStepPos,int nPosNum, 
                      float *pFeaNeg, int nFeaStepNeg,int nNegNum) = 0;    
    /*label can be 0 or 1, or a probility*/ 
    virtual int detect(float *pFea, float *pLabel)=0;  

protected:
};
