#progma once
#include "CWeakLearner.hpp"


class CBoost
{
public:
    CBoost(){}
    virtual ~CBoost(){}

private:
    CWeakLearner *m_weakClassifier;
    int  m_nMaxLayer;
}
