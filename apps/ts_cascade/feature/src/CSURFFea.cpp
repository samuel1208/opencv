#include "CSURFFea.hpp"

/*
  so many parameters, need to improve.
 */
/*
template <typename T> CSURFFea<T> :: 
CSURFFea(THandle hMem, unsigned char* pGrayImg, 
         int nWidthStep, int nWidht, int nHeight, 
         int *pBlockSizeList, int nCellWidth, int nCellHeight, 
         int nBlockMoveStep, int nFeaDim , int nFeaNum)//:
    // CFeature(hMem, nFeaDim, nFeaNum)
{
    m_pGrayImg = pGrayImg;
    m_nGrayWidthStep = nWidthStep;    

    m_pInterImg = SAM_NULL;
    m_nInterImg_width = nWidth;
    m_nInterImg_height = nHeight;


    m_pBlockSizeList = pBlockSizeList;
    m_nBlockTypeNum = sizeof(pBlockSizeList) / 2;
    m_nCellWidth = nCellWidth;
    m_nCellHeight = nCellHeight;
    m_nBlockMoveStep = nBlockMoveStep;
}



template <typename T>  int CSURFFea<T> :: 
initial()
{
    if(  
           (SAM_NULL == m_pGrayImg)  
         ||(m_nInterImg_width <= 0)
         ||(m_nInterImg_height <=0)
      )
        return SAM_ERROR;

    // alloc memory for intergral image
    int size = 0;
    size = m_nIterImg_width * m_nInterImg_height * m_nFeaDim * sizeof(unsigned int);
    m_pInterImg = (int *)TMemAlloc(m_hMem, size);

    if(SAM_NULL == m_pInterImg)
        return SAM_ERROR;

    TMemSet(m_pInterImg, 0, size);

    // alloc memory for feature
    size = m_nFeaDim * m_nFeaNum * sizeof(T);
    m_pFea = (T *)TMemAlloc(m_hMem, size);

    if(SAM_NULL == m_pFea)
        return SAM_ERROR;

    TMemSet(m_pInterImg, 0, size);
    return SAM_OK;
}


template <typename T>  void CSURFFea<T> :: 
unInitial()
{
    if(m_pFea) 
    {
        TMemFree(m_hMem, m_pFea);
        m_pFea = TNull;
    }
    if(m_pInterImg) 
    {
        TMemFree(m_hMem, m_pInterImg);
        m_pImterImg = TNull;
    }
}

template <typename T>  int CSURFFea<T> :: 
extractInterImg()
{
    
    return SAM_OK;
}

template <typename T>  int CSURFFea<T> :: 
getFea(T **ppFea, int *pFeaDim, int *pFeaNum, TRECT rWin)
{

    *ppFea = m_pFea;
    *pFeaDim = m_nFeaDim;
    *pFeaNum = m_nFeaNum;
    return SAM_OK;
}
*/
