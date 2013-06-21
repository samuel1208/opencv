#pragma once
#include "CWeakLearner.hpp"

enum LR_Solve{ITERATIVE_SCALING=0};

/*
  nIterNum is for:
      Iterative Scaling
 */

class CLRWeakLearner : public CWeakLearner
{
public :
    CLRWeakLearner(
                   int nFeaDim,
                   int nIterNum = 10000,
                   LR_Solve nSolveMethod = ITERATIVE_SCALING );
    
    virtual ~CLRWeakLearner();

    //if set by out side, don't call this
    virtual int  initial();
    virtual void unInitial();

    virtual int  solve(float *pFeaPos, int nFeaStepPos,int nPosNum, 
                       float *pFeaNeg, int nFeaStepNeg,int nNegNum);
    /*
      PARA:
          pOutput : probability of the class
          pLabel  : 0 negative, 1 positive
    */ 
    virtual int detect(float *pFea, float *pOutput, int *pLabel);

private:    

    //For Iterative Scaling Algorithm
    int preCalCulateForIT(double *pSum, float *pR12, float *pMaxSum,
                          float *pFeaPos, int nFeaStepPos,int nPosNum, 
                          float *pFeaNeg, int nFeaStepNeg,int nNegNum);
    
    int solve_IT(float *pFeaPos, int nFeaStepPos,int nPosNum, 
                 float *pFeaNeg, int nFeaStepNeg,int nNegNum);

private:
    float  *m_pAlpha;    
    int     m_nFeaDim;
    
    /*-1: unset; 0:alloc by inside; 1:set by inside; 2:set by outside*/
    char    m_bAlphaFlag;    
    int     m_nIterNum;
    LR_Solve m_nSolveMethod;
    
};
