#include <iostream>
#include <fstream>
#include <exception>
#include <glog/logging.h>
#include "BiChuLi.h"
#include "KxianChuLi.h"
#include "Utils.h"

using namespace std;
const size_t BI_MIN_K_NUM(4);
bool jumpEmpty(const vector<vector<Kxian>::iterator>& tempKxianList, KDirection direction) {
    return false;
}
bool ifChengbiNew(vector<vector<Kxian>::iterator>& tempKxianList, KDirection direction, bool updatePreBi = false, float preBiEnd = 0.0)
{
    if (tempKxianList.size() < BI_MIN_K_NUM && !jumpEmpty(tempKxianList, direction))
    {
        // 如果连4根K线都没有的话肯定不是笔
        LOG(INFO) << "Not a Bi, because less than 5 Kxian, size:" << tempKxianList.size();
        return false;
    }
    if (direction == KDirection::KD_DOWN) {
        size_t validKxianNum(1);
        auto currDi = tempKxianList[0]->di;
        for (size_t i = 1; i < tempKxianList.size(); ++i) {
            if (tempKxianList[i]->di < currDi) {
                currDi = tempKxianList[i]->di;
                ++validKxianNum;
            }
        }
        if (currDi <= preBiEnd) {
            updatePreBi = true;
        }
        if (validKxianNum > BI_MIN_K_NUM) {
            LOG(INFO) << "found 5 down Kxian, return true" << "start: (" << getDate(tempKxianList[0]->dateBegin) << ":" << getDate(tempKxianList[0]->dateEnd) << ") " << tempKxianList[0]->gao << ":" << tempKxianList[0]->di
                << "end: (" << getDate(tempKxianList[tempKxianList.size() - 1]->dateBegin) << ":" << getDate(tempKxianList[tempKxianList.size() - 1]->dateEnd) << ") " << tempKxianList[tempKxianList.size() - 1]->gao << ":" << tempKxianList[tempKxianList.size() - 1]->di;
            return true;
        }
    }
    else if (direction == KDirection::KD_UP) {
        size_t validKxianNum(1);
        auto currGao = tempKxianList[0]->gao;
        for (size_t i = 1; i < tempKxianList.size(); ++i) {
            if (tempKxianList[i]->gao > currGao) {
                currGao = tempKxianList[i]->gao;
                ++validKxianNum;
            }
        }
        if (currGao >= preBiEnd) {
            updatePreBi = true;
        }
        if (validKxianNum > BI_MIN_K_NUM) {
            LOG(INFO) << "found 5 up Kxian, return true" << "start: (" << getDate(tempKxianList[0]->dateBegin) << ":" << getDate(tempKxianList[0]->dateEnd) << ") " << tempKxianList[0]->gao << ":" << tempKxianList[0]->di
                << "end: (" << getDate(tempKxianList[tempKxianList.size() - 1]->dateBegin) << ":" << getDate(tempKxianList[tempKxianList.size() - 1]->dateEnd) << ") " << tempKxianList[tempKxianList.size() - 1]->gao << ":" << tempKxianList[tempKxianList.size() - 1]->di;
            return true;
        }
    }
    LOG(INFO) << "not found 5 down/up Kxian, return false" << "start: (" << getDate(tempKxianList[0]->dateBegin) << ":" << getDate(tempKxianList[0]->dateEnd) << ") " << tempKxianList[0]->gao << ":" << tempKxianList[0]->di
        << "end :(" << getDate(tempKxianList[tempKxianList.size() - 1]->dateBegin) << ":" << getDate(tempKxianList[tempKxianList.size() - 1]->dateEnd) << ") " << tempKxianList[tempKxianList.size() - 1]->gao << ":" << tempKxianList[tempKxianList.size() - 1]->di << "size" << tempKxianList.size();
    return false;
}

bool ifChengbi(vector<std::vector<Kxian>::iterator> &tempKxianList, KDirection direction)
{
    return ifChengbiNew(tempKxianList, direction);
}

void BiChuLi::handle(vector<Kxian>& kxianList)
{
    vector<vector<Kxian>::iterator> tempKxianList; // 临时未成笔K线的保存
    for (vector<Kxian>::iterator iter = kxianList.begin(); iter != kxianList.end(); iter++)
    {
        if (this->biList.empty())
        {
            // 第一笔生成中，也是假设第一笔是向上的
            Bi bi;
            bi.fangXiang = KDirection::KD_UP;
            bi.kaiShi = (*iter).kaiShi;
            bi.jieShu = (*iter).jieShu;
            bi.gao = (*iter).gao;
            bi.di = (*iter).di;
            bi.kxianList.push_back(*iter);
            this->biList.push_back(bi);
            LOG(INFO) << "New bi start " << iter->dumpLogInfo();
        }
        else
        {
            if (this->biList.back().fangXiang == KDirection::KD_UP)
            {
                // 上一笔是向上笔
                if ((*iter).gao >= this->biList.back().gao)
                {
                    // 到了一个高点，先判断tempKxianList里边有没有创新低
                    // 如果创新低了，说明调整下上一笔：废掉上一个向上笔，下降笔直接延伸到底点
                    auto itLow = Kxian::getMaxMin(tempKxianList, KDirection::KD_DOWN);
                    LOG(INFO) << "need adjust check, preLow " << (*itLow)->di << "Bilist Size: " << this->biList.size();
                    if (this->biList.size() > 1) {
                        auto preBiLow = this->biList[this->biList.size() - 2].di;
                        auto direction = this->biList[this->biList.size() - 2].fangXiang;
                        LOG(INFO) << "need adjust check, direction " << int(direction) << " preBilow: " << preBiLow;
                        if (direction == KDirection::KD_DOWN && preBiLow > (*itLow)->di) {
                            LOG(INFO) << "Adjust a bi, discard a up bi start Kxian" << this->biList.back().kxianList[0].dumpLogInfo() << "end Kxian " << this->biList.back().kxianList.back().dumpLogInfo();
                            auto biTemp = this->biList.back();
                            this->biList.pop_back();
                            iter = *itLow;
                            for (auto it = biTemp.kxianList.begin(); it != biTemp.kxianList.end(); it++)
                            {
                                this->biList.back().kxianList.push_back(*it);
                            }
                            LOG(INFO) << "tempKxianList have Kxian, try to add them to pre bi start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                            for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                            {
                                LOG(INFO) << "erease ing";
                                this->biList.back().kxianList.push_back(**it);
                                //tempKxianList.erase(tempKxianList.begin()); // TODO change to list;
                                if (it == itLow) {                                    
                                    break;
                                }
                            }
                            this->biList.back().jieShu = (*itLow)->jieShu;
                            this->biList.back().di = (*itLow)->di;
                            //tempKxianList.erase(tempKxianList.begin());
                            tempKxianList.clear();
                            continue;
                        }
                    }
                    // 否则，向上笔继续延续
                    this->biList.back().jieShu = (*iter).jieShu;
                    this->biList.back().gao = (*iter).gao;
                    LOG(INFO) << "up bi continue" << iter->dumpLogInfo();
                    if (tempKxianList.size() > 0)
                    {
                        LOG(INFO) << "tempKxianList have Kxian, try to add them to pre bi start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                        for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                        {
                            this->biList.back().kxianList.push_back(**it);
                        }
                        tempKxianList.clear();
                    }
                    this->biList.back().kxianList.push_back(*iter);
                }
                else
                {

                    tempKxianList.push_back(iter);
                    // 有没有成新的向下笔
                    if (ifChengbi(tempKxianList, KDirection::KD_DOWN))
                    {
                        Bi bi;
                        bi.fangXiang = KDirection::KD_DOWN;
                        bi.kaiShi = this->biList.back().jieShu;
                        bi.jieShu = tempKxianList.back()->jieShu;
                        bi.di = tempKxianList.back()->di;
                        bi.gao = this->biList.back().gao;
                        for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                        {
                            bi.kxianList.push_back(**it);
                        }
                        LOG(INFO) << "new down Bi created start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                        tempKxianList.clear();
                        this->biList.push_back(bi);
                    }
                }
            }
            else if (this->biList.back().fangXiang == KDirection::KD_DOWN)
            {
                // 上一笔是向下笔
                if ((*iter).di <= this->biList.back().di)
                {
                    // 到了一个低点，先判断tempKxianList里边有没有创新高
                   // 如果创新高了，说明调整下上一笔：废掉上一个向下笔，上升笔直接延伸到高点
                    auto itHigh = Kxian::getMaxMin(tempKxianList, KDirection::KD_UP);
                    LOG(INFO) << "need adjust check, preHigh " << (*itHigh)->gao << "Bilist Size: " << this->biList.size();
                    if (this->biList.size() > 1) {
                        auto preBiHigh = this->biList[this->biList.size() - 2].gao;
                        auto direction = this->biList[this->biList.size() - 2].fangXiang;
                        if (direction == KDirection::KD_UP && preBiHigh < (*itHigh)->gao) {
                            LOG(INFO) << "Adjust a bi, discard a down bi start Kxian" << this->biList.back().kxianList[0].dumpLogInfo() << "end Kxian " << this->biList.back().kxianList.back().dumpLogInfo();
                            auto biTemp = this->biList.back();
                            this->biList.pop_back();
                            iter = *itHigh;
                            for (auto it = biTemp.kxianList.begin(); it != biTemp.kxianList.end(); it++)
                            {
                                this->biList.back().kxianList.push_back(*it);
                            }
                            LOG(INFO) << "tempKxianList have Kxian, try to add them to pre bi start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                            for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                            {
                                LOG(INFO) << "erease ing";
                                this->biList.back().kxianList.push_back(**it);
                                //tempKxianList.erase(tempKxianList.begin()); // TODO change to list;
                                if (it == itHigh) {
                                    break;
                                }
                            }
                            this->biList.back().jieShu = (*itHigh)->jieShu;
                            this->biList.back().gao = (*itHigh)->gao;
                            //tempKxianList.erase(tempKxianList.begin());
                            tempKxianList.clear();
                            continue;
                        }
                    }
                    // 向下笔继续延续
                    this->biList.back().jieShu = (*iter).jieShu;
                    this->biList.back().di = (*iter).di;
                    LOG(INFO) << "down bi continue" << iter->dumpLogInfo();
                    if (tempKxianList.size() > 0)
                    {
                        for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                        {
                            this->biList.back().kxianList.push_back(**it);
                        }
                        LOG(INFO) << "tempKxianList have Kxian, try to add them to pre bi start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                        tempKxianList.clear();
                    }
                    this->biList.back().kxianList.push_back(*iter);
                }
                else
                {
                    tempKxianList.push_back(iter);
                    // 有没有成新的向上笔
                    if (ifChengbi(tempKxianList, KDirection::KD_UP))
                    {
                        Bi bi;
                        bi.fangXiang = KDirection::KD_UP;
                        bi.kaiShi = this->biList.back().jieShu;
                        bi.jieShu = tempKxianList.back()->jieShu;
                        bi.gao = tempKxianList.back()->gao;
                        bi.di = this->biList.back().di;
                        for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                        {
                            bi.kxianList.push_back(**it);
                        }
                        LOG(INFO) << "new up Bi created start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                        tempKxianList.clear();
                        this->biList.push_back(bi);

                    }
                }
            }
        }
        if (tempKxianList.size() >= BI_MIN_K_NUM)
        {
            if (this->biList.back().fangXiang == KDirection::KD_UP)
            {
                if (ifChengbi(tempKxianList, KDirection::KD_DOWN))
                {
                    Bi bi;
                    bi.fangXiang = KDirection::KD_DOWN;
                    bi.kaiShi = this->biList.back().jieShu;
                    bi.jieShu = tempKxianList.back()->jieShu;
                    bi.di = tempKxianList.back()->di;
                    bi.gao = this->biList.back().gao;
                    for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                    {
                        bi.kxianList.push_back(**it);
                    }
                    LOG(INFO) << "new up Bi created start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                    tempKxianList.clear();
                    this->biList.push_back(bi);
                }
            }
            else if (this->biList.back().fangXiang == KDirection::KD_DOWN)
            {
                if (ifChengbi(tempKxianList, KDirection::KD_UP))
                {
                    Bi bi;
                    bi.fangXiang = KDirection::KD_UP;
                    bi.kaiShi = this->biList.back().jieShu;
                    bi.jieShu = tempKxianList.back()->jieShu;
                    bi.gao = tempKxianList.back()->gao;
                    bi.di = this->biList.back().di;
                    for (auto it = tempKxianList.begin(); it != tempKxianList.end(); it++)
                    {
                        bi.kxianList.push_back(**it);
                    }
                    LOG(INFO) << "new down Bi created start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
                    tempKxianList.clear();
                    this->biList.push_back(bi);
                }
            }
        }
    }
}
