#include"CRealBoost.hpp"
#include "CLRWeakLearner.hpp"
#include <iostream>
#include <common.h>
#include <math.h>
#include <stdio.h>


CRealBoost::
CRealBoost(BOOST_PARA &boost_para, 
           int nPosNum, int nNegNum,
           int nFeaDim, int nFeaNum):
    CBoost(boost_para, nFeaDim, nFeaNum)
{
    m_nPosNum = nPosNum;
    m_nNegNum = nNegNum;
    m_fScorePre = 0;
}

int CRealBoost::
initial()
{
    int i; 
    unInitial();
    m_ppWeakClassifier =(CWeakLearner**) TMemAlloc(memHandle(), m_nFeaNum * sizeof(CWeakLearner*));
    if(TNull == m_ppWeakClassifier)
        return SAM_ERROR;

    TMemSet(m_ppWeakClassifier, 0, m_nFeaNum * sizeof(CWeakLearner*));
    for (i=0; i<m_nFeaNum; i++)
    {
        if(WC_LR == m_boostPara.wcMethod)
        {
            m_ppWeakClassifier[i] = new CLRWeakLearner(m_nFeaDim);
            if(SAM_OK != m_ppWeakClassifier[i]->initial())
                return SAM_ERROR;
        }
        else 
            return SAM_ERROR;
    }
    
    m_pWeight = (float *)TMemAlloc(memHandle(), (m_nPosNum+m_nNegNum) * sizeof(float));
    if(TNull == m_pWeight)
        return SAM_ERROR;

    TMemSet(m_pWeight, 0, (m_nPosNum+m_nNegNum) * sizeof(float));

    return SAM_OK;
}
void CRealBoost::
unInitial()
{
    int i;
    if(m_ppWeakClassifier)
    {
        for (i=0; i<m_nFeaNum; i++)
        {
            if( m_ppWeakClassifier[i])
            {
                delete m_ppWeakClassifier[i];
                m_ppWeakClassifier[i] = TNull;
            }
            else
                break;
        }
        TMemFree(memHandle(), m_ppWeakClassifier);
        m_ppWeakClassifier = TNull;
    }

    for (std::vector<CWeakLearner*>::iterator iter_wl=m_vWeakClassifierPool.begin();iter_wl!=m_vWeakClassifierPool.end();iter_wl++)  
    {
        //iterator is a point 
       delete (*iter_wl);
    }  
    m_vWeakClassifierPool.clear();
    m_vFeatureIndexPool.clear();

    if(m_pWeight)
    {
        TMemFree(memHandle(), m_pWeight);
        m_pWeight = TNull;
    }
    return ;
}

int CRealBoost::
train(float *pFeaPos, int nStepPos,
      float *pFeaNeg, int nStepNeg)
{
    int rVal = SAM_OK;
    int t = 0, nMaxLayer = m_boostPara.nMaxLayer;
    int i;
    float *pSampleOutput = TNull,*pSampleBaseOutput = TNull ;

    pSampleOutput = (float *) TMemAlloc(memHandle(), sizeof(float)*(m_nPosNum + m_nNegNum));
    pSampleBaseOutput = (float *) TMemAlloc(memHandle(), sizeof(float)*(m_nPosNum + m_nNegNum));
    if((TNull == pSampleOutput) || (TNull == pSampleBaseOutput))
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }

    rVal = initialWeight(m_nPosNum, m_nNegNum);
    if(SAM_OK != rVal)
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }

    for(t=0; t<nMaxLayer; t++)
    {
        int optimal_index = -1;
        // get all weak Learner
        for(i=0; i<m_nFeaNum; i++)
        {
            if(SAM_OK != m_ppWeakClassifier[i]->solve(pFeaPos+i*m_nFeaDim,
                                                      nStepPos, m_nPosNum,
                                                      pFeaNeg+i*m_nFeaDim,
                                                      nStepNeg, m_nNegNum))
            {
                rVal = SAM_ERROR;
                goto SAM_EXIT;
            }
        }
        //get min weakLearner
        optimal_index = findOptimalWL_ROC(pSampleBaseOutput, pSampleOutput,
                                          pFeaPos, nStepPos,
                                          pFeaNeg, nStepNeg);
        if(optimal_index < 0)
        {
            printf("The ROC score can't converge , Break the boost loop\n");
            //set the threshhold 
            if((0 != m_boostPara.fTP) || ( 0 != m_boostPara.fFP))
            {            
                int bIsOk = 0;
                if(SAM_OK != isTpAndFpOK(pSampleOutput,
                                         pFeaPos,nStepPos,
                                         pFeaNeg, nStepNeg, &bIsOk))
                {
                    rVal = SAM_ERROR;
                    goto SAM_EXIT;
                }
            }
            break;
        }
        
        m_vWeakClassifierPool.push_back((m_ppWeakClassifier[optimal_index]));
        if(WC_LR == m_boostPara.wcMethod)
        {
            m_ppWeakClassifier[optimal_index] = new CLRWeakLearner(m_nFeaDim);
        }
        
        m_vFeatureIndexPool.push_back(optimal_index);

        updateWeight(pFeaPos, nStepPos, pFeaNeg, nStepNeg); 

        //check the fp and tp
        if((0 != m_boostPara.fTP) || ( 0 != m_boostPara.fFP))
        {
            int bIsOk = 0;
            if(SAM_OK != isTpAndFpOK(pSampleOutput,
                                     pFeaPos,nStepPos,
                                     pFeaNeg, nStepNeg, &bIsOk))
            {
                 rVal = SAM_ERROR;
                 goto SAM_EXIT;
            }
            if(1 == bIsOk)
                break;
        }
        
    }
    
    rVal = SAM_OK;
 SAM_EXIT:
    if(pSampleOutput)TMemFree(memHandle(), pSampleOutput);
    if(pSampleBaseOutput)TMemFree(memHandle(), pSampleBaseOutput);
    return rVal;
}

int CRealBoost::
getRocScore(float *pOutput, double *pScore)
{
    const int step = 20;
    const float th_begin = 0.5f;
    const float th_end = -0.5f;
    const float th_step = (th_begin- th_end)/step;
    
    float  th = th_begin;
    double fp = 0, tp=0, fp_pre=0, tp_pre=0;
    int i=0, s=0;
    float *ptr;
    double score = 0;

    if((TNull == pOutput) || (TNull == pScore))
        return SAM_ERROR;
    
    for(s=0; s<=step; s++, th-= th_step)
    {
        //get fp and tp
        tp = fp =0;
        ptr = pOutput;
        for(i=0; i<m_nPosNum; i++)
        {
            if(ptr[i]>=th)
                tp+= m_pWeight[i];
                //if add 1, every stage is the same
                //tp += 1;
        }
        ptr += m_nPosNum ;
        for(i=0; i<m_nNegNum; i++)
        {
            if(ptr[i]>=th)
                //fp += 1;
                fp += m_pWeight[i+m_nPosNum];
        }        
        tp /= 0.5F;
        fp /= 0.5f;
        if(fp != fp_pre)
        {
            score += (tp_pre+tp)*(fp-fp_pre)/2;
        } 
        fp_pre = fp;
        tp_pre = tp;
    }
    *pScore = score;
    return SAM_OK;
}

int CRealBoost::
findOptimalWL_ROC(float *pSamplePreOutput, 
                  float *pSampleOutput,
                  float *pFeaPos, int nStepPos,
                  float *pFeaNeg, int nStepNeg)
{
    int i=0, j=0;
    float fOutput;
    float *ptr = TNull;
    int index = -1;
    double score = 0;

    if((TNull == pSamplePreOutput)
       ||(TNull == pSampleOutput)
       ||(TNull == pFeaPos)
       ||(TNull == pFeaNeg))
        return -1;

    TMemSet(pSamplePreOutput, 0, sizeof(float)*(m_nPosNum + m_nNegNum));
    TMemSet(pSampleOutput, 0, sizeof(float)*(m_nPosNum + m_nNegNum));
    //first get the pre-weaklearner output 
    ptr = pFeaPos;
    for(i=0; i<m_nPosNum; i++)
    {
        std::vector<CWeakLearner*>::iterator iter_wl; 
        std::vector<int>::iterator iter_index = m_vFeatureIndexPool.begin();  
        for (iter_wl=m_vWeakClassifierPool.begin();iter_wl!=m_vWeakClassifierPool.end();iter_wl++)  
        {
            (*iter_wl)->detect(ptr+(*iter_index)*m_nFeaDim,&fOutput, TNull);
            iter_index++;
            pSamplePreOutput[i] += fOutput;
        }  
        ptr += nStepPos;
    }
    ptr = pFeaNeg;
    for(i=0; i<m_nNegNum; i++)
    {
        std::vector<CWeakLearner*>::iterator iter_wl; 
        std::vector<int>::iterator iter_index = m_vFeatureIndexPool.begin();  
        for (iter_wl=m_vWeakClassifierPool.begin();iter_wl!=m_vWeakClassifierPool.end();iter_wl++)  
        {
            (*iter_wl)->detect(ptr+(*iter_index)*m_nFeaDim,&fOutput, NULL);
            iter_index++;
            pSamplePreOutput[i+m_nPosNum] += fOutput;
        }  
        ptr += nStepNeg;
    }

    //add the new wl 
    for(i=0; i<m_nFeaNum; i++)
    {
        ptr = pFeaPos;
        for (j=0; j<m_nPosNum; j++)
        {
            m_ppWeakClassifier[i]->detect(ptr+i*m_nFeaDim, pSampleOutput+j, TNull);
            pSampleOutput[j] += pSamplePreOutput[j];
            ptr += nStepPos;            
        }

        ptr = pFeaNeg;
        for(j=0; j<m_nNegNum; j++)
        {
            m_ppWeakClassifier[i]->detect(ptr+i*m_nFeaDim,
                                          pSampleOutput+j+m_nPosNum, TNull);
            pSampleOutput[j+m_nPosNum] += pSamplePreOutput[j+m_nPosNum];
            ptr += nStepNeg;
        }

        //get ROC score
        getRocScore(pSampleOutput, &score);
        if(score>m_fScorePre)
        {
            m_fScorePre = score;
            index =i;
        }        
    }
    return index;    
}


int CRealBoost::
initialWeight(int nPosNum, int nNegNum)
{
    int i = 0;
    float *ptr = TNull;
    float weight = 0;

    if(TNull == m_pWeight)
        return SAM_ERROR;
    
    ptr = m_pWeight;
    weight = 0.5f / nPosNum;
    for(i=0; i<nPosNum; i++)
        ptr[i] = weight;

    ptr = m_pWeight + nPosNum;
    weight = 0.5f / nNegNum;
    for(i=0; i<nNegNum; i++)
        ptr[i] = weight;
    
    return SAM_OK;
}

int CRealBoost::
updateWeight(float *pFeaPos, int nStepPos,
             float *pFeaNeg, int nStepNeg)
{
    int i = 0;
    float *ptr = TNull;
    double sum = 0;    
    float  fOutput=0;
    std::vector<CWeakLearner*>::reverse_iterator iter_wl = m_vWeakClassifierPool.rbegin();
    std::vector<int>::reverse_iterator iter_index = m_vFeatureIndexPool.rbegin();  
        

    ptr = pFeaPos;
    for(i=0; i<m_nPosNum; i++)
    {        
        (*iter_wl)->detect(ptr+(*iter_index)*m_nFeaDim, &fOutput, TNull);
        m_pWeight[i] = m_pWeight[i] * exp(-fOutput);        
        ptr += nStepPos;
    }
    
    ptr = pFeaNeg;
    for(i=0; i<m_nNegNum; i++)
    {
        (*iter_wl)->detect(ptr+(*iter_index)*m_nFeaDim, &fOutput, TNull);
        m_pWeight[i+m_nPosNum] = m_pWeight[i+m_nPosNum] * exp(fOutput);        
        ptr += nStepNeg;
    }

    //normalize
    sum = 0;
    for(i=0; i<m_nNegNum+m_nPosNum; i++)
    {
        sum = m_pWeight[i];
    }
    for(i=0; i<m_nNegNum+m_nPosNum; i++)
    {
        m_pWeight[i]  = (float)(m_pWeight[i]/sum);
    }
    
    return SAM_OK;
}

int CRealBoost::
isTpAndFpOK(float *pSampleOutput,
            float *pFeaPos, int nStepPos,
            float *pFeaNeg, int nStepNeg, int *bIsOk)
{
    int i, j;
    float val1, val2, val3;
    float fOutput = 0;
    float *ptr = TNull;
    TMemSet(pSampleOutput, 0, sizeof(float)*(m_nPosNum + m_nNegNum));
    //first get the pre-weaklearner output 
    ptr = pFeaPos;
    
    for(i=0; i<m_nPosNum; i++)
    {
        std::vector<CWeakLearner*>::iterator iter_wl; 
        std::vector<int>::iterator iter_index = m_vFeatureIndexPool.begin();  
        for (iter_wl=m_vWeakClassifierPool.begin();iter_wl!=m_vWeakClassifierPool.end();iter_wl++)  
        {
            (*iter_wl)->detect(ptr+(*iter_index)*m_nFeaDim,&fOutput, TNull);
            iter_index++;
            pSampleOutput[i] += fOutput;
        }  
        ptr += nStepPos;
    }
    ptr = pFeaNeg;
    pSampleOutput +=m_nPosNum; 
    for(i=0; i<m_nNegNum; i++)
    {
        std::vector<CWeakLearner*>::iterator iter_wl; 
        std::vector<int>::iterator iter_index = m_vFeatureIndexPool.begin();  
        for (iter_wl=m_vWeakClassifierPool.begin();iter_wl!=m_vWeakClassifierPool.end();iter_wl++)  
        {
            (*iter_wl)->detect(ptr+(*iter_index)*m_nFeaDim,&fOutput, TNull);
            iter_index++;
            pSampleOutput[i] += fOutput;
        }  
        ptr += nStepNeg;
    }
    
    //sort the positive output
    pSampleOutput -=m_nPosNum;
    for(i=0; i<m_nPosNum; i++)
    {
        for(j=i+1; j<m_nPosNum; j++ )
        {
            val1 = pSampleOutput[i];
            val2 = pSampleOutput[j];
            if(val2 > val1)
            {
                val3 = val1;
                val1 = val2;
                val2 = val3;
            }
            pSampleOutput[i] = val1;
            pSampleOutput[j] = val2;
            
        }
    }
    m_fThresholdOfBoost = pSampleOutput[(int)(m_boostPara.fTP*m_nPosNum)];

    // cal the fp;
    pSampleOutput +=m_nPosNum;
    j=0;
    for (i=0; i<m_nNegNum; i++)
    {
        if(pSampleOutput[i]>=m_fThresholdOfBoost)
            j++;
    }
    if(j>m_nNegNum * m_boostPara.fFP)
        *bIsOk = 0;
    else 
        *bIsOk = 1;

    return SAM_OK;    
}

int CRealBoost::
detect(float *pFea, float *pOutput, int *pLabel)
{
    float fOutput = 0;
    double output_total = 0;

    std::vector<CWeakLearner*>::iterator iter_wl; 
    std::vector<int>::iterator iter_index = m_vFeatureIndexPool.begin();  

    if(
       (TNull == pFea)
       ||((TNull == pOutput)&&(TNull==pLabel))
       )
        return SAM_ERROR;

    for (iter_wl=m_vWeakClassifierPool.begin();iter_wl!=m_vWeakClassifierPool.end();iter_wl++)  
    {
        (*iter_wl)->detect(pFea+(*iter_index)*m_nFeaDim,&fOutput, TNull);
        iter_index++;
        output_total += fOutput;
    }  
    
    if(pOutput)
        *pOutput = (float)output_total;
    if(pLabel)
    {
        if(output_total >= m_fThresholdOfBoost)
            *pLabel = 1;
        else 
            *pLabel = 0;
    }
    return SAM_OK;
}

CRealBoost::
~CRealBoost()
{
    unInitial();
}
