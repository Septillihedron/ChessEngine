#pragma once
#include <stdint.h>

#define PositionValue uint16_t
#define MIN_VALUE 0
#define MAX_VALUE 

#include "BoardRepresentation.h"
#include "NNUE.h"

template <bool Maximizing>
PositionValue AlphaBeta(u8 depth);
