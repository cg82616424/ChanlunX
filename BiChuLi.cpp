#include <iostream>
#include <fstream>
#include <exception>
#include <glog/logging.h>
#include "BiChuLi.h"
#include "KxianChuLi.h"
#include "Utils.h"

using namespace std;
const size_t BI_MIN_K_NUM(4);
bool jumpEmpty(const vector<Kxian>& tempKxianList, KDirection direction) {
    return false;
}
bool ifChengbiNew(vector<Kxian>& tempKxianList, KDirection direction, bool updatePreBi = false, float preBiEnd = 0.0)
{
    if (tempKxianList.size() < BI_MIN_K_NUM && !jumpEmpty(tempKxianList, direction))
    {
        // 如果连4根K线都没有的话肯定不是笔
        LOG(INFO) << "Not a Bi, because less than 5 Kxian, size:" << tempKxianList.size();
        return false;
    }
    if (direction == KDirection::KD_DOWN) {
        size_t validKxianNum(1);
        auto currDi = tempKxianList[0].di;
        for (size_t i = 1; i < tempKxianList.size(); ++i) {
            if (tempKxianList[i].di < currDi) {
                currDi = tempKxianList[i].di;
                ++validKxianNum;
            }
        }
        if (currDi <= preBiEnd) {
            updatePreBi = true;
        }
        if (validKxianNum > BI_MIN_K_NUM) {
            LOG(INFO) << "found 5 down Kxian, return true" << "start: (" << getDate(tempKxianList[0].dateBegin) << ":" << getDate(tempKxianList[0].dateEnd) << ") " << tempKxianList[0].gao << ":" << tempKxianList[0].di
                << "end: (" << getDate(tempKxianList[tempKxianList.size() - 1].dateBegin) << ":" << getDate(tempKxianList[tempKxianList.size() - 1].dateEnd) << ") " << tempKxianList[tempKxianList.size() - 1].gao << ":" << tempKxianList[tempKxianList.size() - 1].di;
            return true;
        }
    }
    else if (direction == KDirection::KD_UP) {
        size_t validKxianNum(1);
        auto currGao = tempKxianList[0].gao;
        for (size_t i = 1; i < tempKxianList.size(); ++i) {
            if (tempKxianList[i].gao > currGao) {
                currGao = tempKxianList[i].gao;
                ++validKxianNum;
            }
        }
        if (currGao >= preBiEnd) {
            updatePreBi = true;
        }
        if (validKxianNum > BI_MIN_K_NUM) {
            LOG(INFO) << "found 5 up Kxian, return true" << "start: (" << getDate(tempKxianList[0].dateBegin) << ":" << getDate(tempKxianList[0].dateEnd) << ") " << tempKxianList[0].gao << ":" << tempKxianList[0].di
                << "end: (" << getDate(tempKxianList[tempKxianList.size() - 1].dateBegin) << ":" << getDate(tempKxianList[tempKxianList.size() - 1].dateEnd) << ") " << tempKxianList[tempKxianList.size() - 1].gao << ":" << tempKxianList[tempKxianList.size() - 1].di;
            return true;
        }
    }
    LOG(INFO) << "not found 5 down Kxian, return true" << "start: (" << getDate(tempKxianList[0].dateBegin) << ":" << getDate(tempKxianList[0].dateEnd) << ") " << tempKxianList[0].gao << ":" << tempKxianList[0].di
        << "end :(" << getDate(tempKxianList[tempKxianList.size() - 1].dateBegin) << ":" << getDate(tempKxianList[tempKxianList.size() - 1].dateEnd) << ") " << tempKxianList[tempKxianList.size() - 1].gao << ":" << tempKxianList[tempKxianList.size() - 1].di << "size" << tempKxianList.size();
    return false;
}

bool ifChengbi(vector<Kxian> &tempKxianList, KDirection direction)
{
    return ifChengbiNew(tempKxianList, direction);
    if (tempKxianList.size() < BI_MIN_K_NUM)
    {
        LOG(INFO) << "Not a Bi, because less than 5 Kxian, size:" << tempKxianList.size();
        // 如果连4根K线都没有的话肯定不是笔
        return false;
    }
    if (direction == KDirection::KD_DOWN)
    {
        // 是不是向下成笔
        // 先找有没有下降K线
        unsigned int i = 2;
        unsigned int valid_count(0);
        while (true)
        {
            for (; i < tempKxianList.size(); i++)
            {
                if (tempKxianList.at(i).di < tempKxianList.at(i - 1).di && tempKxianList.at(i - 1).di < tempKxianList.at(i - 2).di)
                {
                    // 找到下降K线
                    break;
                }
            }
            if (i >= tempKxianList.size())
            {
                // 下降K线都没找到
                LOG(INFO) << "Not Found a down Kxian, return false, index: " << i << "size:" << tempKxianList.size();
                return false;
            }
            // 找前面的最低价
            float zuiDiJia = tempKxianList.at(i).di;
            // 如果出现了更低价，这个笔就成立了
            for (unsigned int j = i + 1; j < tempKxianList.size(); j++)
            {
                if (tempKxianList.at(j).di < zuiDiJia)
                {
                    LOG(INFO) << "Found a down Kxian, return true, index: " << j << "size:" << tempKxianList.size();
                    return true;
                }
            }
            i = i + 1;
        }
    }
    else if (direction == KDirection::KD_UP)
    {
        // 是不是向上成笔
        // 先找有没有上升K线
        unsigned int i = 2;
        while (true)
        {
            for (; i < tempKxianList.size(); i++)
            {
                if (tempKxianList.at(i).gao > tempKxianList.at(i - 1).gao && tempKxianList.at(i - 1).gao > tempKxianList.at(i - 2).gao)
                {
                    // 找到上升K线
                    break;
                }
            }
            if (i >= tempKxianList.size())
            {
                // 上升K线都没找到
                LOG(INFO) << "Not Found a up Kxian, return false, index: " << i << "size:" << tempKxianList.size();
                return false;
            }
            // 找前面的最高价
            float zuiGaoJia = tempKxianList.at(i).gao;
            // 如果出现了更高价，这个笔就成立了
            for (unsigned int j = i + 1; j < tempKxianList.size(); j++)
            {
                if (tempKxianList.at(j).gao > zuiGaoJia)
                {
                    LOG(INFO) << "Found a up Kxian, return true, index: " << j << "size:" << tempKxianList.size();
                    return true;
                }
            }
            i = i + 1;
        }
    }
    LOG(INFO) << "Return false by default";
    return false;
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
                    // 向上笔继续延续
                    this->biList.back().jieShu = (*iter).jieShu;
                    this->biList.back().gao = (*iter).gao;
                    LOG(INFO) << "up bi continue" << iter->dumpLogInfo();
                    if (tempKxianList.size() > 0)
                    {
                        LOG(INFO) << "tempKxianList have Kxian, try to add them to pre bi start Kxian:" << tempKxianList.begin()->dumpLogInfo() << " end Kxian:" << tempKxianList.back().dumpLogInfo();
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
                        LOG(INFO) << "new down Bi created start Kxian:" << (*tempKxianList.begin())->dumpLogInfo() << " end Kxian:" << (*tempKxianList.back()).dumpLogInfo();
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
