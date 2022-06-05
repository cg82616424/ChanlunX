#include "KxianChuLi.h"
#include "Utils.h"
#include <sstream>
#include <string>
#include <glog/logging.h>
using namespace std;

string Kxian::dumpLogInfo() {
    stringstream ss;
    ss << "date: (" << dateBegin << ":" << dateEnd << ") " << "price: (" << di << ":" << gao << ")" << "direction:" << static_cast<int>(this->kDirection) << "flags: " << this->kaiShi << ":" << this->jieShu << ":" << this->zhongJian;
    return ss.str();
}

void KxianChuLi::add(float gao, float di, int date)
{
    KxianRaw kxianRaw;
    kxianRaw.gao = gao;
    kxianRaw.di = di;
    kxianRaw.date = date;
    // 保存原始K线
    this->kxianRawList.push_back(kxianRaw);
    if (this->kxianList.empty())
    {
        // 第一根K线先假设方向为上
        Kxian kxian;
        kxian.gao = gao;
        kxian.di = di;
        kxian.dateBegin = date;
        kxian.dateEnd = date;
        kxian.kDirection = KDirection::KD_UP;
        kxian.kaiShi = 0;
        kxian.jieShu = 0;
        kxian.zhongJian = 0;
        this->kxianList.push_back(kxian);
    }
    else
    {
        if (gao > this->kxianList.back().gao && di > this->kxianList.back().di)
        {
            // 向上
            Kxian kxian;
            kxian.gao = gao;
            kxian.di = di;
            kxian.kDirection = KDirection::KD_UP;
            kxian.kaiShi = this->kxianList.back().jieShu + 1;
            kxian.jieShu = kxian.kaiShi;
            kxian.zhongJian = kxian.kaiShi;
            kxian.dateBegin = date;
            kxian.dateEnd = date;
            // 新K线
            this->kxianList.push_back(kxian);
        }
        else if (gao < this->kxianList.back().gao && di < this->kxianList.back().di)
        {
            // 向下
            Kxian kxian;
            kxian.gao = gao;
            kxian.di = di;
            kxian.kDirection = KDirection::KD_DOWN;
            kxian.kaiShi = this->kxianList.back().jieShu + 1;
            kxian.jieShu = kxian.kaiShi;
            kxian.zhongJian = kxian.kaiShi;
            //date
            kxian.dateBegin = date;
            kxian.dateEnd = date;
            // 新K线
            this->kxianList.push_back(kxian);
        }
        else if (gao <= this->kxianList.back().gao && di >= this->kxianList.back().di)
        {
            // 前包含
            LOG(INFO) << "pre Contain K xian: date"<< getDate(date) << " High: " << gao << " Low: " << di;
            //date
            this->kxianList.back().dateEnd = date;
            if (this->kxianList.back().kDirection == KDirection::KD_UP)
            {
                //this->kxianList.back().di = di;
            }
            else
            {
                //this->kxianList.back().gao = gao;
            }
            this->kxianList.back().jieShu = this->kxianList.back().jieShu + 1;
        }
        else
        {
            // 后包含
            LOG(INFO) << "post Contain K xian: date" << getDate(date) << " High: " << gao << " Low: " << di;
            //date
            this->kxianList.back().dateEnd = date;
            if (this->kxianList.back().kDirection == KDirection::KD_UP)
            {
                //this->kxianList.back().gao = gao;
            }
            else
            {
                //this->kxianList.back().di = di;
            }
            this->kxianList.back().jieShu = this->kxianList.back().jieShu + 1;
            this->kxianList.back().zhongJian = this->kxianList.back().jieShu;
        }
    }
}
