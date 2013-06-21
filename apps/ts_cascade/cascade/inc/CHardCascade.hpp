#pragma once

#include "CCascade.hpp"

class CHardCascade : public  CCascade
{
public :
    CHardCascade(TRAIN_PARA &trainPara);
    virtual ~CHardCascade();
    virtual int train();
};
