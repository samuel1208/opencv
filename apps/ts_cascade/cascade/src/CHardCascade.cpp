#include "CHardCascade.hpp"

CHardCascade::
CHardCascade( TRAIN_PARA &trainPara)
    :CCascade(trainPara)
{
}



int CHardCascade::
train()
{
    int rVal = SAM_OK;
    int l=0;
    int nMaxLayer = m_trainPara.nMaxLayer;
    int bIsSampleEmpty = 0;
    int nNegNumStill = 0;
    BOOST_PARA boost_para;
    int nFeaSize = m_nFeaDim*m_nFeaNum;
    int nPosNumInEachLayer=0, nNegNumInEachLayer=0, nNegGotNum=0;
    boost_para.fTP = m_trainPara.fTP;
    boost_para.fFP = m_trainPara.fFP;
    boost_para.nMaxLayer = 20;
    boost_para.wcMethod = m_trainPara.wcMethod;
    
    nNegNumStill = 0;
    nPosNumInEachLayer = m_trainPara.nPosNumInEachLayer;
    nNegNumInEachLayer = m_trainPara.nNegNumInEachLayer;
    for(l=0; l<nMaxLayer; l++)
    {
        CBoost *boost = TNull;
        
        if(0 == l)
        {
            rVal = getFeaFor1stLayer();
            if(SAM_OK != rVal)
                goto SAM_EXIT;
            // the number may be changed in getFeaFor1stLayer function
            nPosNumInEachLayer = m_trainPara.nPosNumInEachLayer;
            nNegNumInEachLayer = m_trainPara.nNegNumInEachLayer;
        }
        else
        {
            //update the negatives sample
            rVal = getNegFeaFromImgList(m_pNegFea + nNegNumStill*nFeaSize,
                                        nNegNumInEachLayer-nNegNumStill,
                                        &nNegGotNum, 1);
            if(SAM_OK != rVal)
                goto SAM_EXIT;

            if(nNegGotNum != (nNegNumInEachLayer-nNegNumStill))
                bIsSampleEmpty = 1;
            
            if(nNegNumStill + nNegGotNum <  nNegNumInEachLayer/8)
                break;
            else
                nNegNumInEachLayer = nNegNumStill + nNegGotNum;
        }

        if(BOOST_REAL == m_trainPara.boostMethod)
            boost = new CRealBoost(boost_para, 
                                   nPosNumInEachLayer,
                                   nNegNumInEachLayer,
                                   m_nFeaDim, m_nFeaNum);
        rVal = boost->initial();
        if(SAM_OK != rVal)
            goto SAM_EXIT;

        rVal = boost->train(m_pPosFea, nFeaSize,
                            m_pNegFea, nFeaSize);
        if(SAM_OK != rVal)
            goto SAM_EXIT;
        
        m_vStrongClassifierPool.push_back(boost);

        if(1 == bIsSampleEmpty)
            break;

        // remove the negative fea which can be classified correctly 
        rVal = removeCorrectNegFea(m_pNegFea, nNegNumInEachLayer, &nNegNumStill);
        if(SAM_OK != rVal)
            goto SAM_EXIT;
        nNegNumStill = nNegNumInEachLayer - nNegNumStill;
    }

    rVal = SAM_OK;
 SAM_EXIT:
    return rVal;
}

CHardCascade::
~CHardCascade()
{
    unInitial();
}
