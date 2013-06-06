

#include "CLRWeakLearner.hpp"
#include <stdio.h>

int main()
{
    CWeakLearner *wl = NULL;

    float pFeaPos[9] = {1,3,3,2,4,3,5,9,3};
    float pFeaNeg[16] = {5,3,3,4,7,4,3,4,10,9,3,4, 12, 11,3,4};

    float test[2] = {1,1};
    float label;

    wl = new CLRWeakLearner(NULL, 2);

    wl->initial();
    wl->solve(pFeaPos, 3, 3, pFeaNeg,4,4);
    wl->detect(test,&label);
    printf("%f", label);
    wl->unInitial();
    return 0;
    
}
