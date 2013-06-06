#pragma once

#include "CCascade.hpp"

enum weekClassifier{LR=0, LSVM=1, Real=2};

class CHardCascade : public  CCascade
{
public :
    CHardCascade(){}
    virtual ~CHardCascade(){}

    virtual int train();

private:
    float  m_fTPrInEachStage;
    float  m_fFPrInEachStage;
}
