#include "CCascade.hpp"

CCascade::
CCascade()
    {
        TMemSet(&m_trainPara, 0, sizeof(TRAIN_PARA));
        m_pCFea = TNull;
        m_pPosFea = TNull;
        m_pNegFea = TNull;
        m_nFeaDim = 0;
        m_nFeaNum = 0;        
        
        m_fPosArdFile = TNull;
        m_fNegArdFile = TNull;
        m_fNegListFile = TNull;
        m_nPosArdImgNum = 0; // = filesize/ imageSize
        m_nNegArdImgNum = 0;
    }

CCascade::
CCascade(TRAIN_PARA &trainPara)
    {
        m_trainPara = trainPara;
        m_pCFea = TNull;
        m_pPosFea = TNull;
        m_pNegFea = TNull;
        m_nFeaDim = 0;
        m_nFeaNum = 0;        
        
        m_fPosArdFile = TNull;
        m_fNegArdFile = TNull;
        m_fNegListFile = TNull;
        m_nPosArdImgNum = 0; // = filesize/ imageSize
        m_nNegArdImgNum = 0;      
    }

int CCascade::
openSampleFile()
{
    int imgSize = 0;
    int fileSize = 0;
  
    if(
       (TNull != m_fPosArdFile)
       ||(TNull != m_fNegArdFile)
       ||(TNull !=  m_fNegListFile)
       )
        return SAM_ERROR;

    imgSize = m_trainPara.nWidth * m_trainPara.nHeight * m_trainPara.nChannel;
    
    m_fPosArdFile = fopen(m_trainPara.sPosArdPath.c_str(), "r");
    if(TNull ==  m_fPosArdFile)
    {
        printf("ERROR:: Open positive ard file failed\n");
        return SAM_ERROR;
    }
    fseek(m_fPosArdFile, 0, SEEK_END);
    fileSize = ftell(m_fPosArdFile);
    fseek(m_fPosArdFile, 0, SEEK_SET);
    m_nPosArdImgNum = fileSize / imgSize;    
    
    m_fNegArdFile = fopen(m_trainPara.sNegArdPath.c_str(), "r");
    if(TNull ==  m_fNegArdFile)
    {
        printf("ERROR:: Open Negative ard file failed\n");
        return SAM_ERROR;
    }
    fseek(m_fNegArdFile, 0, SEEK_END);
    fileSize = ftell(m_fNegArdFile);
    fseek(m_fNegArdFile, 0, SEEK_SET);
    m_nNegArdImgNum = fileSize / imgSize;

    m_fNegListFile = fopen(m_trainPara.sNegListPath.c_str(), "r");
    if(TNull ==  m_fNegArdFile)
    {
        printf("ERROR:: Open Negative ard file failed\n");
        return SAM_ERROR;
    }
    return SAM_OK;
}

void CCascade::
closeSampleFile()
{    
    if(m_fPosArdFile)
    {
        fclose(m_fPosArdFile);
        m_fPosArdFile = TNull;
    }
    if(m_fNegArdFile)
    {
        fclose(m_fNegArdFile);
        m_fPosArdFile = TNull;
    }
    if(m_fNegListFile)
    {
        fclose(m_fNegListFile);
        m_fNegListFile = TNull;
    }
}

int CCascade::
initialFeaBuf()
{
    int size = 0;
    if(FEA_HOG == m_trainPara.fea_method)
    {
        if(
           (TNull == m_trainPara.pBlockSizeList)
           ||(m_trainPara.nBlockSizeListNum<1)
           )
            return SAM_ERROR;
            
        m_pCFea = new  CHOGFea(m_trainPara.nWidth, 
                               m_trainPara.nHeight, 
                               m_trainPara.pBlockSizeList,
                               m_trainPara.nBlockSizeListNum);
        
        if(SAM_OK != m_pCFea->initial())
            return SAM_ERROR;
        m_pCFea->getFeaSize(
                            {0,0, m_trainPara.nWidth, m_trainPara.nHeight}, 
                                &m_nFeaDim, &m_nFeaNum
                            );             
    }
    else 
        return SAM_ERROR;

    size = m_trainPara.nPosNumInEachLayer * m_nFeaDim * m_nFeaNum * sizeof(float);
    m_pPosFea = (float *)TMemAlloc(memHandle(), size);
    if(TNull == m_pPosFea)
        return SAM_ERROR;

    size = m_trainPara.nNegNumInEachLayer * m_nFeaDim * m_nFeaNum * sizeof(float);
    m_pNegFea = (float *)TMemAlloc(memHandle(), size);
    if(TNull == m_pNegFea)
        return SAM_ERROR;   
    return SAM_OK;
}

void CCascade::
unInitialFeaBuf()
{
    if(m_pCFea)
    {
        delete m_pCFea;
        m_pCFea = TNull;
    }

    if(m_pPosFea)
    {
        TMemFree(m_hBuf,m_pPosFea);
        m_pPosFea = TNull;
    }
    if(m_pNegFea)
    {
        TMemFree(m_hBuf,m_pNegFea);
        m_pNegFea = TNull;
    }
}


int  CCascade::
getFeaFor1stLayer()
{
    int rVal = SAM_OK;
    int nWidth  = m_trainPara.nWidth;
    int nHeight = m_trainPara.nHeight;
    int nChannel= m_trainPara.nChannel;
    int nWidthStep = nWidth * nChannel;
    int imgSize = nWidth*nHeight*nChannel;
    unsigned char *pImg = TNull;
    int nFeaSize = m_nFeaDim*m_nFeaNum;
    int imgNum = 0;

    float *pFea = TNull;
    int i=0;
    if(m_nPosArdImgNum < m_trainPara.nPosNumInEachLayer)
    {
        printf("\n-----------------------------------------------------\n");
        printf("nPosNumInEachLayer is : %d", m_trainPara.nPosNumInEachLayer);
        printf("The image number of positive Ard file is : %d\n", m_nPosArdImgNum);
        printf("Set nPosNumInEachLayer to  PosArdImgNum \n");
        printf("-----------------------------------------------------\n\n");
        m_trainPara.nPosNumInEachLayer = m_nPosArdImgNum; 
    }
    
    if(imgSize < 1)
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }
    
    pImg = (unsigned char *)TMemAlloc(memHandle(), imgSize);
    if(TNull == pImg)
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }

    //get positive feature
    pFea = m_pPosFea;
    imgNum = m_trainPara.nPosNumInEachLayer;
    for(i=0; i<imgNum; i++)
    {
        if( 1 != fread(pImg, imgSize, 1, m_fPosArdFile))
	    continue;
        rVal = m_pCFea->extractInterImg(pImg, nWidthStep,
                                            nWidth, nHeight, nChannel);
        if(SAM_OK != rVal)
            goto SAM_EXIT;

        rVal = m_pCFea->getFea(pFea, {0,0,nWidth, nHeight}, NULL);
        if(SAM_OK != rVal)
            goto SAM_EXIT;
        pFea += nFeaSize;
    }
    
    // get Negative Feature
    pFea = m_pNegFea;
    if(m_trainPara.nNegNumInEachLayer > m_nNegArdImgNum)
        imgNum =  m_nNegArdImgNum;
    else 
        imgNum = m_trainPara.nNegNumInEachLayer;

    m_nNegArdImgNum -= m_trainPara.nNegNumInEachLayer;

    for(i=0; i<imgNum; i++)
    {
        if(1 != fread(pImg, imgSize, 1, m_fNegArdFile))
	    continue;
        rVal = m_pCFea->extractInterImg(pImg, nWidthStep,
                                            nWidth, nHeight, nChannel);
        if(SAM_OK != rVal)
            goto SAM_EXIT;

        rVal = m_pCFea->getFea(pFea, {0,0,nWidth, nHeight}, NULL);
        if(SAM_OK != rVal)
            goto SAM_EXIT;
        pFea += nFeaSize;
    }

    //need to complement the negative feature if not enough
    if(m_nNegArdImgNum < 0)
    {
        int nFeaGotNum = 0;
        rVal = getNegFeaFromImgList(pFea, m_trainPara.nNegNumInEachLayer-imgNum, 
                                    &nFeaGotNum, 0);
        if(SAM_OK != rVal)
            goto SAM_EXIT;

        if(nFeaGotNum != m_trainPara.nNegNumInEachLayer-imgNum)
        {
            printf("WARNING:: Can't get enough negative sample for first stage\n");
            m_trainPara.nNegNumInEachLayer = nFeaGotNum + imgNum;
        }
    }        

 SAM_EXIT:
    if(pImg) TMemFree(memHandle(), pImg);
    return rVal;
}

int CCascade::
getNegFeaFromImgList(float *pNegFea, int nFeaNum,
                     int *pFeaGotNum, int bIsNeedDetect)
{
    int rVal = SAM_OK;
    char filePath[1024] = {0};
    unsigned char *pImg_ARD = TNull;
    IplImage *pImg = TNull;
    IplImage *pImg_template = TNull;
    IplImage *pImgGray_template = TNull, *pImg_tmp = TNull;
    int nWidth_template = m_trainPara.nWidth;
    int nHeight_template = m_trainPara.nHeight;
    int nChannel_template =  m_trainPara.nChannel;
    int nImgSize_template = nWidth_template
                            *nHeight_template 
                            *nChannel_template;
    int nFeaSize = m_nFeaNum*m_nFeaDim;
    float *pFea = pNegFea;
    int nFeaGotNum = 0, nFeaNeedNum = nFeaNum;
    
    if(nFeaNum <1)
        return SAM_ERROR;

    pImg_template = cvCreateImage(cvSize(nWidth_template, nHeight_template),
                                  8, 3);
    pImgGray_template = cvCreateImage(cvSize(nWidth_template, nHeight_template),
                                  8, 1);
    if((TNull == pImg_template) || (TNull==pImgGray_template))
    {
        rVal = SAM_ERROR;
        goto SAM_EXIT;
    }
    
    // TODO :: get sample from ard file first    
    if(m_nNegArdImgNum>0)
    {
        int nLabel = -1;
	pImg_ARD = (unsigned char *)TMemAlloc(memHandle(), nImgSize_template);
	if(TNull == pImg_ARD)
	    goto SAM_EXIT;
	
	while(!feof(m_fNegArdFile))
	{
	    if(1 != fread(pImg_ARD, nImgSize_template, 1, m_fNegArdFile))
	        continue;
	    rVal = m_pCFea->extractInterImg(pImg_ARD, nWidth_template,
					    nWidth_template,
					    nHeight_template,
					    nChannel_template);
	    if(SAM_OK != rVal)
	        goto SAM_EXIT;

	    rVal = m_pCFea->getFea(pFea,
				   {0,0,nWidth_template, nHeight_template}, NULL);
	    if(SAM_OK != rVal)
	        goto SAM_EXIT;

	    rVal = detect(pFea, &nLabel);
	    if(SAM_OK != rVal)
	        goto SAM_EXIT;

	    if(1 == nLabel)
	    {
	        nFeaNum -- ;
	        pFea += nFeaSize;
	    }
	    m_nNegArdImgNum --;
	}
    }

    while(!feof(m_fNegListFile))
    {
        int breakStatus = 0;
        int scale  = 0;
        int nBiWidth_template = nWidth_template/2;
        int nBiHeight_template = nHeight_template/2;
        int nWidth_win = 0;
        int nHeight_win = 0;
        
        TMemSet(filePath, 0, 1024);
        fgets(filePath, 1024, m_fNegListFile);
        if(strlen(filePath) < 4)
            continue;
        if('\n' == filePath[strlen(filePath)-1])
            filePath[strlen(filePath)-1] = 0;

        // 1 read color, -1 read original channel, 0 forth read gray
        pImg = (IplImage*)cvLoadImage(filePath, 1);
        if(TNull == pImg)
            continue;
        
        //skip the small image
        scale = TMIN(pImg->width/nWidth_template, pImg->height/nHeight_template);
        if(scale < 1)
            continue;
        
        nWidth_win  = scale*nWidth_template;
        nHeight_win = scale*nHeight_template;

        for(int i=0; i<4; i++)
        {

            while(1)
            {
                int nStep_x  = nWidth_win;
                int nStep_y  = nHeight_win;
                int nOffset_x = i*(nWidth_win/4);
                int nOffset_y = i*(nHeight_win/4);
                
                if(
                   (nWidth_win<nWidth_template) 
                   || (nHeight_win<nHeight_template)
                   )
                    break;
                rVal = getNegFeasFromImageWithFixWindow(pImg, pImg_template,
							pImgGray_template,
							pFea,
							nOffset_x, nOffset_y,
							nStep_x,   nStep_y,
							nWidth_win, nHeight_win,
							nFeaNum, &nFeaGotNum,
							bIsNeedDetect);
                if(SAM_OK != rVal)
                    goto SAM_EXIT;
                
                nFeaNum -= nFeaGotNum;
                if(0 == nFeaNum)
                    goto SAM_EXIT;
                nWidth_win -= nBiWidth_template;
                nHeight_win -=nBiHeight_template;
            }
        }        
        if(pImg)
        {
            cvReleaseImage(&pImg); 
            pImg = TNull;
        }
    }

    rVal = SAM_OK;
 SAM_EXIT:
    if(pImg_ARD)
    {      
	TMemFree(memHandle(), pImg_ARD);
	pImg_ARD = TNull;
    }
    if(pImg)
    {
        cvReleaseImage(&pImg); 
        pImg = TNull;
    }
    if(pImg_template)
    {
        cvReleaseImage(&pImg_template); 
        pImg_template = TNull;
    }
    if(pImgGray_template)
    {
        cvReleaseImage(&pImgGray_template); 
        pImg = TNull;
    }
    if(SAM_OK == rVal)
        *pFeaGotNum = nFeaNeedNum - nFeaNum;
    else
        *pFeaGotNum = 0;
    return rVal;    
}

int CCascade ::
getNegFeasFromImageWithFixWindow(IplImage *pImg_src, 
                              IplImage *pImg_template,
                              IplImage *pImgGray_template,
                              float     *pFea,
                              int nOffset_x, int nOffset_y,
                              int nStep_x,   int nStep_y,
                              int nWidth_win, int nHeight_win,
                              int nNeededFeaNum, int *pFeaGotNum,
                              int bIsNeedDetect)
{
    int rVal = SAM_OK;
    IplImage *pImg_tmp=TNull;
    int nWidth_src=0, nHeight_src=0;
    int nWidth_template=0, nHeight_template=0;
    int w, h;
    int nFeaCount = 0;
    int nFeaSize = m_nFeaDim * m_nFeaNum;
    int bIsFeaOk = 0;
    float *_pFea = pFea;

    if(
       (TNull == pImg_src)
       ||(TNull == pImg_template)
       ||(TNull == pImgGray_template)
       ||(TNull == pFea)
       ||(nNeededFeaNum <1)
       )
    {
        rVal =  SAM_ERROR;
        goto SAM_EXIT;
    }
    
    nWidth_src = pImg_src->width;
    nHeight_src = pImg_src->height;
    nWidth_template = pImg_template->width;
    nHeight_template = pImg_template->height;
    
    _pFea = pFea;
    for(h=nOffset_y; h<=nHeight_src-nStep_y; h+=nStep_y)
    {
        for(w=nOffset_x; w<=nWidth_src-nStep_x; w+=nStep_x)
        {
            bIsFeaOk = 0;
            cvSetImageROI(pImg_src, cvRect(w,h,nWidth_win, nHeight_win));
            cvResize(pImg_src, pImg_template, CV_INTER_LINEAR);
            if(FEA_HOG == m_trainPara.fea_method)
            {
                cvCvtColor(pImg_template, pImgGray_template, CV_RGB2GRAY);
                pImg_tmp = pImgGray_template;
            }
            else 
                pImg_tmp = pImg_template;
            
            rVal = m_pCFea->extractInterImg((unsigned char*)pImg_tmp->imageData, 
                                            pImg_tmp->widthStep, 
                                            pImg_tmp->width, 
                                            pImg_tmp->height, 
                                            pImg_tmp->nChannels);
            if(SAM_OK != rVal)            
                goto SAM_EXIT;   
                    
            rVal = m_pCFea->getFea(_pFea, {0,0,nWidth_template, nHeight_template}, NULL);
            if(SAM_OK != rVal)            
                goto SAM_EXIT;   
                    
            if(1 == bIsNeedDetect)
            {
                int nLabel = -1;
		rVal = detect(_pFea, &nLabel);
		if(SAM_OK != rVal)
		    goto  SAM_EXIT;
		if(1 == nLabel)
		    bIsFeaOk = 1;
            }
            else 
                bIsFeaOk = 1;

            if(1 == bIsFeaOk)
            {
                nFeaCount ++;
                _pFea += nFeaSize;
            }
            if(nFeaCount == nNeededFeaNum)
            {
                rVal = SAM_OK;
                goto SAM_EXIT;
            }            
        }
    }
    rVal = SAM_OK;
 SAM_EXIT:
    *pFeaGotNum = nFeaCount;
    return rVal;
}

int CCascade::
removeCorrectNegFea(float *pFea, int nFeaNum, int *pRemovedNum)
{
    std::vector<CBoost *>::iterator boost_iter;
    float *_pFea = TNull;
    int nFeaSize = m_nFeaDim * m_nFeaNum;
    int nRemovedNum = 0;
    int i; 

    if((TNull == pFea)||(TNull == pRemovedNum))
        return SAM_ERROR;

    if(m_vStrongClassifierPool.size()<1)
        return SAM_ERROR;
        
    _pFea = pFea;
    
    for(; nFeaNum>0; nFeaNum--)
    {
        /*
	  TODO :: just need to check the last strong classifier, 
	          need to improve
        */
	int nLabel = -1;
	if(SAM_OK != detect(_pFea, &nLabel))
	    return SAM_ERROR;
	if(0 == nLabel)
	{
	  // remove the fea
	    {
	        float *__pFea = _pFea;
		int j=0;
		// TODO : remove fea outside 
		for(j=0; j<nFeaNum; j++)
		{
		    TMemCpy(__pFea, __pFea+nFeaSize, nFeaSize);
		    __pFea += nFeaSize;
		}
	    }
	    nRemovedNum ++; 
	}
	else
	    _pFea += nFeaSize; 
    }

    *pRemovedNum = nRemovedNum;
    return SAM_OK;
}


void CCascade:: 
releaseBoostVector()
{
    std::vector<CBoost *>::iterator boost_iter = m_vStrongClassifierPool.begin();
    for(; boost_iter != m_vStrongClassifierPool.end(); boost_iter++)
    {
        (*boost_iter)->unInitial();
        delete (*boost_iter);
    }
    m_vStrongClassifierPool.clear();
}

int CCascade::
initial()
{    
    int rVal = SAM_OK;
    
    rVal = openSampleFile();
    if(SAM_OK != rVal)
        return rVal;

    rVal = initialFeaBuf();
    if(SAM_OK != rVal)
        return rVal;    
    
    return SAM_OK;
}

void CCascade::
unInitial()
{    
    closeSampleFile();
    unInitialFeaBuf();
    releaseBoostVector();
}

int CCascade::
detect(float *pFea, int* pLabel)
{
    int nLabel = -1;
    std::vector<CBoost *>::iterator Boost_iter;

    if((TNull == pFea) || (TNull == pLabel))
        return SAM_ERROR;
  
    Boost_iter = m_vStrongClassifierPool.begin();
    for(;Boost_iter!= m_vStrongClassifierPool.end(); Boost_iter++)
    {
        if(SAM_OK !=(*Boost_iter)->detect(pFea,TNull, &nLabel))
	{
	    *pLabel = -1;
	    return SAM_ERROR;
	}
	if(0 == nLabel)
	    break;
    }
    
    *pLabel = nLabel;
    return SAM_OK;
}
