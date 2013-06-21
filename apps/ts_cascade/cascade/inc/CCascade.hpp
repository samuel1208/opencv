#pragma once

#include "CBoost.hpp"
#include "CRealBoost.hpp"
#include "CFeature.hpp"
#include "CHOGFea.hpp"
#include "CWeakLearner.hpp"
#include "CLRWeakLearner.hpp"
#include "base-object.hpp"

#include "tmem.h"
#include "tcomdef.h"
#include "stdio.h"
#include <string>
#include <iostream>
#include "opencv/cv.h"
#include "opencv/highgui.h"
typedef struct _tag_TRAIN_PARA
{   
    //cascade para
    int  nPosNumInEachLayer;
    int  nNegNumInEachLayer;  
    float fFP;
    float fTP;
    int   nMaxLayer;    
    WC_Method wcMethod;    
    BOOST_Method  boostMethod;

    // Ard File Para
    int  nWidth;
    int  nHeight;
    int  nChannel;    
    std::string sPosArdPath;
    std::string sNegArdPath;
    std::string sNegListPath;        

    //feature para
    FEA_Method fea_method;
    TPOINT *pBlockSizeList;
    int     nBlockSizeListNum;
}TRAIN_PARA;


class CCascade : public Object
{
public :

    CCascade();
    CCascade(TRAIN_PARA &trainPara);

    virtual ~CCascade(){}
    virtual int train()=0;
    virtual int initial();
    virtual void unInitial();

protected:

    // initial and uninitial function
    int  openSampleFile();
    void closeSampleFile();
    int  initialFeaBuf();
    void unInitialFeaBuf();
    void releaseBoostVector();

    int  getFeaFor1stLayer();
    int  getNegFeaFromImgList(float *pNegFea, int nFeaNum,
                              int *pFeaGotNum, int bIsNeedDetect);
    int getFeasFromImageWithFixWindow(IplImage *pImg_src, 
                                      IplImage *pImg_template,
                                      IplImage *pImgGray_template,
                                      float     *pFea,
                                      int nOffset_x, int nOffset_y,
                                      int nStep_x,   int nStep_y,
                                      int nWidth_win, int nHeight_win,
                                      int nNeededFeaNum, int *pFeaGotNum,
                                      int bIsNeedDetect);

    int removeCorrectNegFea(float *pFea, int nFeaNum, int *pRemovedNum);
protected:

    TRAIN_PARA m_trainPara;   
    std::vector<CBoost *> m_vStrongClassifierPool;

    //sample file    
    FILE *m_fPosArdFile;
    FILE *m_fNegArdFile;
    FILE *m_fNegListFile;
    int   m_nPosArdImgNum; // = filesize/ imageSize
    int   m_nNegArdImgNum;

    // feature buffer
    CFeature *m_pCFea;
    float *m_pPosFea;
    float *m_pNegFea;
    int  m_nFeaDim;
    int  m_nFeaNum;
};
