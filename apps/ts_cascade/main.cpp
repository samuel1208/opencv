#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CFeature.hpp"
#include "CSURFFea.hpp"
#include "CHOGFea.hpp"
#include "tcomdef.h"
#include "CRealBoost.hpp"
#include "CLRWeakLearner.hpp"
#include "opencv2/opencv.hpp"
// #include "opencv2/nonfree.hpp"
#include "CBoost.hpp"
#include <iostream>
#include <string>
#include "CCascade.hpp"
#include "CHardCascade.hpp"
using namespace std;
using namespace cv;



int main(int argc, char **argv)
{
    
    int rVal = 0;    
    char *configPath = argv[1];
    
    CCascade *pCascade = TNull;
    TRAIN_PARA train_para;
    TPOINT *pBlockSizeList = TNull;
    
    // //should move to config file
    // TPOINT pBlockSizeList[]= {{12,12}, {16,16}, {20,20}, {24,24},
    //                           {12,24}, {24,12}, {16,24}, {24,16}};
    // float *pPosFea=TNull, *pNegFea=TNull;
    // int nFeaDim=0, nFeaNum = 0, nFeaSize = 0;
    // unsigned char *pImgBuf = NULL;


    //para for analysis the config file
    FileStorage *configFile = new FileStorage(configPath, FileStorage::READ);
    FileNode fNode ;
    FILE *pPosFile = NULL, *pNegFile = NULL;
    int nFileSize;


    if(!configFile->isOpened())
    {
        printf("ERROR:: Cann't open the config file :%s\n", configPath);
        rVal = -1;
        goto SAM_EXIT;
    }

    // read config file
    train_para.sPosArdPath = (string)(*configFile)["posARD"];
    train_para.sNegArdPath = (string)(*configFile)["negARD"];
    train_para.sNegListPath = (string)(*configFile)["negListPath"];
    fNode = (*configFile)["imageSize"];
    train_para.nWidth = fNode["width"];
    train_para.nHeight = fNode["height"];
    train_para.nChannel = fNode["channel"];
  
    
    // read boost para
    fNode = (*configFile)["CascadePara"];
    train_para.nMaxLayer = fNode["maxLayerOfCascade"];
    train_para.fTP = (float)fNode["fTP"];
    train_para.fFP = (float)fNode["fFP"];
    train_para.wcMethod = (WC_Method)((int)(fNode["weakclassifier"]));
    train_para.boostMethod = (BOOST_Method)((int)(fNode["boost"]));
    train_para.nPosNumInEachLayer = fNode["nPosNumInEachLayer"];
    train_para.nNegNumInEachLayer = fNode["nNegNumInEachLayer"];  

    // read feature para
    fNode = (*configFile)["feaPara"];    
    train_para.fea_method = (FEA_Method)((int)(fNode["feaType"]));
    train_para.nBlockSizeListNum = fNode["nBlockListNum"];
    pBlockSizeList = (TPOINT*)TMemAlloc(TNull,train_para.nBlockSizeListNum *sizeof(TPOINT));
    if(TNull == pBlockSizeList)
        goto SAM_EXIT;
    // read block list
    //pBlockSizeList = fode["blockList"];
    {
        int i=0;
        std::vector<int>  temp;
        fNode["blockList"] >> temp;
        if(temp.size() != train_para.nBlockSizeListNum*2)
        {
            printf("The Block List is not matched\n");
            goto SAM_EXIT;
        }
        for(i=0; i<train_para.nBlockSizeListNum; i++)
        {
            pBlockSizeList[i].x = temp[2*i];
            pBlockSizeList[i].y = temp[2*i+1];
        }
        temp.clear();
        train_para.pBlockSizeList = pBlockSizeList;
    }

    pCascade = new CHardCascade(train_para);
    rVal = pCascade->initial();
    if(SAM_OK != rVal)
        goto SAM_EXIT;

    rVal = pCascade->train();
    if(SAM_OK != rVal)
        goto SAM_EXIT;

 SAM_EXIT:
    if(pCascade) delete pCascade;
    if(pBlockSizeList) TMemFree(TNull,pBlockSizeList);
    return 0;
}

