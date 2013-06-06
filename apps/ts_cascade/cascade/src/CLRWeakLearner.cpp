#include "CLRWeakLearner.hpp"
#include "common.h"
#include <math.h>

CLRWeakLearner:: 
CLRWeakLearner(THandle hMem,
               int nFeaDim,
               int nIterNum,
               LR_Solve nSolveMethod ):
    CWeakLearner()
{
    m_hMem = hMem;
    m_nFeaDim = nFeaDim;
    m_nIterNum = nIterNum;
    m_nSolveMethod = nSolveMethod;
    m_bAlphaFlag = -1;
    m_pAlpha = TNull;
}


int CLRWeakLearner:: 
initial()
{
    if(m_nFeaDim<1)
        return SAM_ERROR;

    if((0 == m_bAlphaFlag)&&(1==m_bAlphaFlag))
        unInitial();

    m_pAlpha = (float *)TMemAlloc(m_hMem, (m_nFeaDim)*sizeof(float));
    if(TNull == m_pAlpha)
        return SAM_ERROR;

    m_bAlphaFlag = 0;
    return SAM_OK;    
}

void CLRWeakLearner:: 
unInitial()
{
    if((m_pAlpha)&&(2 != m_bAlphaFlag))
    {
        TMemFree(m_hMem, m_pAlpha);
        m_pAlpha = TNull;
        m_bAlphaFlag = -1;
    }
}

int CLRWeakLearner::
preCalCulateForIT(double *pSum, float *pR12, float *pStep,
                  float *pFeaPos, int nFeaStepPos,int nPosNum, 
                  float *pFeaNeg, int nFeaStepNeg,int nNegNum )
{
    int rVal = SAM_OK;
    float  maxSum=0;
    float  tempVal = 0;
    float  tempSum=0;
    int i,j;
    float *ptr = TNull;
    double *pSumPos = TNull, *pSumNeg=TNull;

    if((TNull == pR12) || (TNull == pStep)|| (TNull == pSum))
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }
    pSumPos = (double *)TMemAlloc(m_hMem, m_nFeaDim*sizeof(double));
    pSumNeg = (double *)TMemAlloc(m_hMem, m_nFeaDim*sizeof(double));

    if((TNull == pSumPos) || (TNull == pSumNeg))
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }

    ptr = pFeaPos;
    for(i=0; i<nPosNum; i++)
    {
        tempSum = 0;
        for(j=0; j<m_nFeaDim; j++)
        {
            tempVal = ptr[j];
            if(tempVal <= 0)
            {
                rVal = SAM_ERROR;
                goto SAM_EXIT;
            }
            tempSum += tempVal;
            pSumPos[j] += tempVal;
        }
        if(tempSum > maxSum)
            maxSum = tempSum;
        ptr += nFeaStepPos;
    }
    
    ptr = pFeaNeg;
    for(i=0; i<nNegNum; i++)
    {
        tempSum = 0;
        for(j=0; j<m_nFeaDim; j++)
        {
            tempVal = ptr[j];
            if(tempVal <= 0)
            {
                rVal = SAM_ERROR;
                goto SAM_EXIT;
            }
            tempSum += tempVal;
            pSumNeg[j] += tempVal;
        }
        if(tempSum > maxSum)
            maxSum = tempSum;
        ptr += nFeaStepNeg;
    }
    
    *pStep = (float)(1.0/maxSum);
    for(j=0; j<m_nFeaDim; j++)
    {
        tempSum = pSumPos[j];
        tempVal = pSumNeg[j];
        pR12[j] = (float)(tempSum / tempVal);
        pSum[j] = tempSum + tempVal;
    }    

 SAM_EXIT:
    if(pSumPos) TMemFree(m_hMem, pSumPos);
    if(pSumNeg) TMemFree(m_hMem, pSumNeg);
    return rVal;
}

int CLRWeakLearner::
solve(float *pFeaPos, int nFeaStepPos,int nPosNum, 
      float *pFeaNeg, int nFeaStepNeg,int nNegNum)
{
    int rVal = SAM_OK;
    if(ITERATIVE_SCALING == m_nSolveMethod)
        rVal =solve_IT(pFeaPos, nFeaStepPos,nPosNum, 
                       pFeaNeg, nFeaStepNeg,nNegNum);
    else
        rVal = SAM_ERROR;  

    if(SAM_OK == rVal)
        m_bAlphaFlag = 1;
    return rVal;
}

int CLRWeakLearner::
solve_IT(float *pFeaPos, int nFeaStepPos,int nPosNum, 
         float *pFeaNeg, int nFeaStepNeg,int nNegNum)
{
    int rVal = SAM_OK;
    int i, j, k;
    float maxErr = 0;
    float oldAlpha = 0, newAlpha = 0;
    float *ptr = TNull;
    float *pR12 = TNull;
    double *pSum = TNull, *pS1 = TNull, *pTemp = TNull, val;
    float step = 0;

    if(
       (TNull== pFeaNeg)||(TNull == m_pAlpha)||(TNull== pFeaPos)
       || (nNegNum<1) || (nPosNum<1) || (m_nFeaDim<1)
      )
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }
  
    pR12 = (float *) TMemAlloc(m_hMem, (m_nFeaDim)*sizeof(float));
    pSum = (double *)TMemAlloc(m_hMem, (m_nFeaDim)*sizeof(double));
    pS1 = (double *)TMemAlloc(m_hMem, (m_nFeaDim)*sizeof(double));
    pTemp = (double *)TMemAlloc(m_hMem, (nNegNum + nPosNum)*sizeof(double));
    if((TNull == pR12) || (TNull == pSum)
       ||(TNull == pS1) || (TNull == pTemp))
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }
    TMemSet(m_pAlpha, 0, (m_nFeaDim)*sizeof(float));

    if(SAM_OK != preCalCulateForIT(pSum, pR12, &step,
                                   pFeaPos, nFeaStepPos,nPosNum, 
                                   pFeaNeg, nFeaStepNeg,nNegNum))
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }

    for(i=0; i<m_nIterNum; i++)
    {
        maxErr = 0;
        //calculate s1
        ptr = pFeaPos;
        for(j=0; j<nPosNum; j++)
        {
            val = 0;
            for(k=0; k<m_nFeaDim; k++)
                val += m_pAlpha[k]*ptr[k];
            pTemp[j] = 1.0/(1+exp(-val));
            ptr += nFeaStepPos;
        }
        ptr = pFeaNeg;
        for(j=0; j<nNegNum; j++)
        {
            val = 0;
            for(k=0; k<m_nFeaDim; k++)
                val += m_pAlpha[k]*ptr[k];
            pTemp[j+nPosNum] = 1.0/(1+exp(-val));
            ptr += nFeaStepNeg;
        }

        for(j=0; j<m_nFeaDim; j++)
        {
            val = 0;
            ptr = pFeaPos +j;
            for(k=0; k<nPosNum; k++)
            {
                val += pTemp[k] * ptr[0];
                ptr += nFeaStepPos;
            }
            ptr = pFeaNeg +j;
            for(k=0; k<nNegNum; k++)
            {
                val += pTemp[k+nPosNum] * ptr[0];
                ptr += nFeaStepNeg;                
            }
            pS1[j] = val;
        }
        
        //
        for(j=0; j<m_nFeaDim; j++)
        {
            double delta = 0;
            delta = pSum[j] / pS1[j] - 1;
            oldAlpha = m_pAlpha[j];
            newAlpha = oldAlpha + step * log(pR12[j] * delta);
            
            val = (float)fabs(oldAlpha - newAlpha);
            if(maxErr < val)
                maxErr = val;     
            m_pAlpha[j] = newAlpha;
        }
        if(maxErr<= 0.00001f)
            break;    
    }

 SAM_EXIT:

    if(pR12) TMemFree(m_hMem, pR12);
    if(pSum) TMemFree(m_hMem, pSum);
    if(pS1) TMemFree(m_hMem, pS1);
    if(pTemp) TMemFree(m_hMem, pTemp);
    return rVal;
}


int CLRWeakLearner::
detect(float *pFea, float *pLabel)
{
    double fLabel = 0;
    int i=0;
    if((TNull == pFea) || (TNull == pLabel) 
       || (-1 == m_bAlphaFlag) || (0 == m_bAlphaFlag))
        return SAM_ERROR;

    for(i=0; i<m_nFeaDim; i++)
        fLabel += m_pAlpha[i] * pFea[i];
    
    fLabel = 1.0/ (1+ exp(-fLabel));
    
    *pLabel = fLabel; 
    
    return SAM_OK;    
}

CLRWeakLearner::
~CLRWeakLearner()
{
    unInitial();
}
