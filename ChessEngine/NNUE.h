#pragma once
#include "AlphaBeta.h"

template <bool Black>
PositionValue NNUEEval(long *pieces, CaslingState otherStates, Location enPassantTarget);

