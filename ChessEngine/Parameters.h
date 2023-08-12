#pragma once
#include <stdint.h>

#define u8 uint8_t

constexpr u8 _Max_Search_Depth_ = 25;
constexpr u8 _History_Length_ = _Max_Search_Depth_ + 1;
constexpr bool _Strict_ = true;
