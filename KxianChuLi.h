#ifndef __KXIANCHULI_H__
#define __KXIANCHULI_H__

#include <vector>
#include <string>
using namespace std;

#pragma pack(push, 1)

enum class KDirection {
    KD_UP = 1, //趋势向上
    KD_DOWN = -1 //趋势向下
};
// 原始K线
struct KxianRaw
{
    float gao{0.0f};
    float di{0.0f};
    int date{ 0 };
};

// 表示合并后的K线
struct Kxian
{
public:
    float gao{0.0f};     // K线高
    float di{0.0f};      // K线低
    int dateEnd{ 0 };    //begin time
    int dateBegin{ 0 };  // end time
    KDirection kDirection{KDirection::KD_UP}; // K线方向
    int kaiShi{0};    // 开始K线坐标
    int jieShu{0};    // 结束K线坐标
    int zhongJian{0};
public:
    string dumpLogInfo();
};

class KxianChuLi
{
public:
    vector<KxianRaw> kxianRawList; // 元素K线表
    vector<Kxian> kxianList;       // 包含处理后的K线表
    void add(float gao, float di, int date); // 添加一根K线高和低进行处理
};

#pragma pack(pop)

#endif
