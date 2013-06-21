#pragma once

#include "CBoost.hpp"


class CRealBoost: public CBoost
{
public:
    CRealBoost(){}
    CRealBoost( BOOST_PARA &boost_para, 
               int nPosNum, int nNegNum,
               int nFeaDim, int nFeaNum);
    virtual ~CRealBoost();
    virtual int initial();
    virtual void unInitial();
    virtual int train(float *pFeaPos, int nStepPos,
                      float *pFeaNeg, int nStepNeg);
    virtual int detect(float *pFea, float *pOutput, int *pLabel);
    

private:
    int initialWeight(int nPosNum, int nNegNum);
    int findOptimalWL_ROC(float *pSamplePreOutput, 
                          float *pSampleOutput,
                          float *pFeaPos, int nStepPos,
                          float *pFeaNeg, int nStepNeg);
    int updateWeight(float *pFeaPos, int nStepPos,
                     float *pFeaNeg, int nStepNeg);
    int isTpAndFpOK(float *pSampleOutput,
                    float *pFeaPos, int nStepPos,
                    float *pFeaNeg, int nStepNeg, int *bIsOk);
    int getRocScore(float *pOutput, double *pScore);
        

private:
    int m_nPosNum;
    int m_nNegNum;
    double m_fScorePre;

};
