#pragma once


class CCascade
{
public :
    CCascade(float tp=0.99999, float fp=0.3, int nMaxLayer=20)
    {
        m_tp = tp;
        m_fp = fp;
        m_nMaxLayer = nMaxLayer;
    }
    virtual ~CCascade(){}
    virtual int train()=0;

protected:

    float m_fp;
    float m_tp;
    int   m_nMaxLayer;    
}
