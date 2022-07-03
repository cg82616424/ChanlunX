#include <iostream>
#include <fstream>
#include <exception>
#include <glog/logging.h>
#include "BiChuLi.h"
#include "KxianChuLi.h"
#include "Utils.h"

using namespace std;
const size_t BI_MIN_K_NUM(4); // 考虑跳空
bool ifChengbi(vector<vector<Kxian>::iterator>& tempKxianList, KDirection direction, vector<Kxian>::iterator begin_it, bool updatePreBi = false, float preBiEnd = 0.0)
{
    bool pre_jump = false;
    auto pre_direc = KDirection::KD_UP;
    if (*tempKxianList.begin() != begin_it) {
        auto pre_k = *tempKxianList.begin() - 1;
        pre_jump = pre_k->jumpty.valid;
        pre_direc = pre_k->jumpty.direc;
    }
    if (tempKxianList.size() < 2) // 考虑跳空
    {
        // 如果连4根K线都没有的话肯定不是笔
        LOG(INFO) << "Not a Bi, because less than 5 Kxian, size:" << tempKxianList.size();
        return false;
    }
    if (direction == KDirection::KD_DOWN) {
        size_t validKxianNum(1);
        if (pre_jump && direction == pre_direc) {
            validKxianNum = 5;
        }
        auto currDi = tempKxianList[0]->di;
        for (size_t i = 1; i < tempKxianList.size(); ++i) {
            if (tempKxianList[i]->di < currDi) {
                currDi = tempKxianList[i]->di;
                ++validKxianNum;
            }
            if (tempKxianList[i]->jumpty.valid && direction == tempKxianList[i]->jumpty.direc) {
                validKxianNum += 4; // 有跳空，并且后边3个K没回补，那么可以直接成笔
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
        if (pre_jump && direction == pre_direc) {
            validKxianNum = 5;
        }
        auto currGao = tempKxianList[0]->gao;
        for (size_t i = 1; i < tempKxianList.size(); ++i) {
            if (tempKxianList[i]->gao > currGao) {
                currGao = tempKxianList[i]->gao;
                ++validKxianNum;
            }
            if (tempKxianList[i]->jumpty.valid && direction == tempKxianList[i]->jumpty.direc) {
                validKxianNum += 4; // 有跳空，并且后边3个K没回补，那么可以直接成笔
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

void BiChuLi::pre_handle(vector<Kxian>& kxianList) {
    size_t depth(3);
    LOG(INFO) << "pre_handle start, size:" << kxianList.size();
    for (auto it = kxianList.begin(); it != kxianList.end(); ++it) {
        auto cur_it = it+1;
        if (it == kxianList.end() || cur_it == kxianList.end()) {
            LOG(INFO) << "end kxianlist founded, break";
            break;
        }
        
        //上升趋势跳空
        if (it->gao < cur_it->di) {
            LOG(INFO) << "May be found a jumpty up" << it->dumpLogInfo() << cur_it->dumpLogInfo();
            size_t counter(0);
            double gao = cur_it->di;
            for (; cur_it != kxianList.end(); ++cur_it) {
                if (it->gao < cur_it->di) {
                    ++counter;
                    gao = std::min(static_cast<float>(gao), cur_it->di);
                }
                else {
                    break;
                }
                if (counter >= depth) {
                    it->jumpty.di = it->gao;
                    it->jumpty.gao = gao;
                    it->jumpty.valid = true;
                    it->jumpty.direc = KDirection::KD_UP;
                    LOG(INFO) << "found a jumpty up " << it->dumpLogInfo();
                    break;
                }
            }
        }
        cur_it = it + 1;
        //下降趋势跳空
        if (it->di > cur_it->gao) {
            LOG(INFO) << "May be found a jumpty down" << it->dumpLogInfo() << cur_it->dumpLogInfo();
            size_t counter(0);
            double di(cur_it->gao);
            for (; cur_it != kxianList.end(); ++cur_it) {
                if (it->di > cur_it->gao) {
                    di = std::max(static_cast<float>(di), cur_it->gao);
                    ++counter;
                }
                else {
                    break;
                }
                if (counter >= depth) {
                    it->jumpty.di = di;
                    it->jumpty.gao = it->di;
                    it->jumpty.valid = true;
                    it->jumpty.direc = KDirection::KD_DOWN;
                    LOG(INFO) << "found a jumpty down " << it->dumpLogInfo();
                    break;
                }
            }
        }
    }
};
void BiChuLi::handle(vector<Kxian>& kxianList)
{
    pre_handle(kxianList);
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
                    if (this->biList.size() > 1) {
                        auto preBiLow = this->biList[this->biList.size() - 2].di;
                        auto direction = this->biList[this->biList.size() - 2].fangXiang;
                        LOG(INFO) << "need adjust check, direction " << int(direction) << " preBilow: " << preBiLow;
                        if (itLow != tempKxianList.end() && direction == KDirection::KD_DOWN && preBiLow > (*itLow)->di) {
                            LOG(INFO) << "need adjust check, preLow " << (*itLow)->di << "Bilist Size: " << this->biList.size();
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
                    if (ifChengbi(tempKxianList, KDirection::KD_DOWN, kxianList.begin()))
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
                    if (this->biList.size() > 1) {
                        auto preBiHigh = this->biList[this->biList.size() - 2].gao;
                        auto direction = this->biList[this->biList.size() - 2].fangXiang;
                        if (itHigh != tempKxianList.end() &&  direction == KDirection::KD_UP && preBiHigh < (*itHigh)->gao) {
                            LOG(INFO) << "Adjust a bi, discard a down bi start Kxian" << this->biList.back().kxianList[0].dumpLogInfo() << "end Kxian " << this->biList.back().kxianList.back().dumpLogInfo();
                            LOG(INFO) << "need adjust check, preHigh " << (*itHigh)->gao << "Bilist Size: " << this->biList.size();
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
                    if (ifChengbi(tempKxianList, KDirection::KD_UP,kxianList.begin()))
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
                if (ifChengbi(tempKxianList, KDirection::KD_DOWN,kxianList.begin()))
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
                if (ifChengbi(tempKxianList, KDirection::KD_UP,kxianList.begin()))
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
