#ifndef __BICHULI_H__
#define __BICHULI_H__

#include <vector>
#include "KxianChuLi.h"

using namespace std;

#pragma pack(push, 1)

struct Bi
{
  KDirection fangXiang{KDirection::KD_UP};           // 笔方向
  int kaiShi{0};              // 笔起点
  int jieShu{0};              // 笔终点
  float gao{0.0};               // 笔最高价
  float di{0.0};                // 笔最低价
  vector<Kxian> kxianList; // 一笔当中的K线
};

class BiChuLi
{
public:
  vector<Bi> biList; // 笔的表格
  void handle(vector<Kxian> &kxianList);
};

#pragma pack(pop)

#endif
