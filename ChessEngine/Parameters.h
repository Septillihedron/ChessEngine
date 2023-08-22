#pragma once
#include "Types.h"

constexpr u8 _Max_Search_Depth_ = 25;
constexpr u8 _History_Length_ = _Max_Search_Depth_ + 1;
constexpr bool _Strict_ = false;
constexpr int _Prefetch_Size_ = 8; // 2^n
constexpr int _Hashed_Positions_Size_ = (32*1024*1024) / sizeof(HashEntry);

