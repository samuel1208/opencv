#pragma once
#include "CWeakLearner.hpp"
#include "base-object.hpp"
#include <vector>

enum BOOST_Method{BOOST_REAL = 1};

typedef struct __BOOST_PARA
{
    int    nMaxLayer;
    float  fTP;
    float  fFP;    
    WC_Method wcMethod;          
}BOOST_PARA, *PBOOSTPARA;



class CBoost : public Object
{
public:
    CBoost()
    {
        m_ppWeakClassifier = TNull;
        m_pWeight = TNull;
        m_nFeaNum = 0;
        m_nFeaDim = 0;
        m_fThresholdOfBoost = 0;
        TMemSet(&m_boostPara, 0, sizeof(BOOST_PARA)) ;   
        m_vWeakClassifierPool.clear();
        m_vFeatureIndexPool.clear();
        
    }
    CBoost(BOOST_PARA &boost_para, int nFeaDim, int nFeaNum)
    {
        m_ppWeakClassifier = TNull;
        m_pWeight = TNull;
        m_nFeaNum = nFeaNum;
        m_nFeaDim = nFeaDim;
        m_fThresholdOfBoost = 0;
        m_boostPara =boost_para ;   
        m_vWeakClassifierPool.clear();
        m_vFeatureIndexPool.clear();
    }
    virtual ~CBoost(){}

    virtual int  initial()=0;
    virtual void unInitial()=0;
    virtual int train(float *pFeaPos, int nStepPos, 
                      float *pFeaNeg, int nStepNeg)=0;
    virtual int detect(float *pFea, float *pOutput, int *pLabel)=0;
    

protected:
    CWeakLearner **m_ppWeakClassifier;
    std::vector<CWeakLearner *> m_vWeakClassifierPool;
    std::vector<int> m_vFeatureIndexPool;
    float *m_pWeight;
    int    m_nFeaNum;
    int    m_nFeaDim;    
    float  m_fThresholdOfBoost;
    BOOST_PARA m_boostPara;
};
