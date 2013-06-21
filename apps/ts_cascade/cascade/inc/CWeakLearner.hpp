#pragma once

#include "tcomdef.h"
#include "tmem.h"
#include "base-object.hpp"

enum WC_Method{WC_LR = 1};

class CWeakLearner : public Object
{
public :
    CWeakLearner(){}
    virtual ~CWeakLearner(){}

    virtual int initial() = 0;
    virtual void unInitial() = 0;

    virtual int solve(float *pFeaPos, int nFeaStepPos,int nPosNum, 
                      float *pFeaNeg, int nFeaStepNeg,int nNegNum) = 0;
    
    /*
      PARA:
          pOutput : probability of the class
          pLabel  : 0 negative, 1 positive
    */ 
    virtual int detect(float *pFea, float *pOutput, int *pLabel)=0;  

protected:
};
