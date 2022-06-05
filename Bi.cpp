#include <iostream>
#include <fstream>
#include <glog/logging.h>
#include "Bi.h"
#include "KxianChuLi.h"
#include "BiChuLi.h"

using namespace std;

std::vector<float> Bi1(int nCount, std::vector<float> pHigh, std::vector<float> pLow, std::vector<int> pDate)
{
    std::vector<float> pOut(nCount);
    KxianChuLi kxianChuLi;
    for (int i = 0; i < nCount; i++)
    {
        kxianChuLi.add(pHigh[i], pLow[i], pDate[i]);
    }
    for (unsigned int i = 1; i < kxianChuLi.kxianList.size(); i++)
    {
        if (kxianChuLi.kxianList.at(i - 1).kDirection != kxianChuLi.kxianList.at(i).kDirection)
        {
            if (kxianChuLi.kxianList.at(i - 1).kDirection == KDirection::KD_UP)
            {
                pOut[kxianChuLi.kxianList.at(i - 1).zhongJian] = 1;
            }
            else if (kxianChuLi.kxianList.at(i - 1).kDirection == KDirection::KD_DOWN)
            {
                pOut[kxianChuLi.kxianList.at(i - 1).zhongJian] = -1;
            }
        }
    }
    if (kxianChuLi.kxianList.back().kDirection == KDirection::KD_UP)
    {
        pOut[kxianChuLi.kxianList.back().zhongJian] = 1;
    }
    else if (kxianChuLi.kxianList.back().kDirection == KDirection::KD_DOWN)
    {
        pOut[kxianChuLi.kxianList.back().zhongJian] = -1;
    }
    return pOut;
}

std::vector<float> Bi2(int nCount, std::vector<float> pHigh, std::vector<float> pLow, std::vector<int> pDate)
{
    std::vector<float> pOut(nCount);
    KxianChuLi kxianChuLi;
    for (int i = 0; i < nCount; i++)
    {
        kxianChuLi.add(pHigh[i], pLow[i], pDate[i]);
    }
    BiChuLi biChuLi;
    biChuLi.handle(kxianChuLi.kxianList);
    for (vector<Bi>::iterator iter = biChuLi.biList.begin(); iter != biChuLi.biList.end(); iter++)
    {
        if ((*iter).fangXiang == KDirection::KD_UP)
        {
            pOut[(*iter).kxianList.back().zhongJian] = 1;
        }
        else if ((*iter).fangXiang == KDirection::KD_DOWN)
        {
            pOut[(*iter).kxianList.back().zhongJian] = -1;
        }
    }
    return pOut;
}
