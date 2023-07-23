#pragma once
#include <stdint.h>

#include "MoveGen.h"

constexpr u8 maxSearchDepth = 25;

#define PositionValue int16_t
constexpr PositionValue MIN_VALUE = INT16_MIN+1+maxSearchDepth;
constexpr PositionValue MAX_VALUE = INT16_MAX;
constexpr PositionValue DRAW_VALUE = 0;
constexpr Move NullMove = { (u8) -1, (u8) -1, (u8) -1 };

#include "Evaluator.h"

inline u8 searchDepth = 0;

typedef struct Variation {
	u8 cmove;
	Move moves[maxSearchDepth];
} Variation;

inline Variation principleVariation;
inline bool searchCanceled = false;

template <bool Black>
Move Search() {
	Variation principleVariationNext = {};
	Move bestMove;
	AlphaBeta<Black>(1, MIN_VALUE, MAX_VALUE, &principleVariationNext);
	principleVariation = principleVariationNext;
	bestMove = principleVariation.moves[0];
	for (u8 i = 2; i <= 5; i++) {
		searchDepth = i;
		PositionValue value = AlphaBeta<Black>(i, MIN_VALUE, MAX_VALUE, &principleVariationNext);
		std::string betsMoveStr = principleVariationNext.moves[0].ToString();
		std::cout << "depth " << (int) i << ": " << betsMoveStr << "(" << (int) value << ")" << std::endl;
		if (searchCanceled) {
			break;
		}
		principleVariation = principleVariationNext;
		bestMove = principleVariation.moves[0];
	}
	principleVariation.cmove -= 2;
	memmove(principleVariation.moves, principleVariation.moves + 2, principleVariation.cmove);
	return bestMove;
}

inline Move Search(bool isBlackTurn) {
	if (isBlackTurn) {
		return Search<true>();
	}
	else {
		return Search<false>();
	}
}

inline u8 moveIndexTempSize[21];
inline u8 moveIndexTemp[21][256];
inline Move movesTemp[256];

template <bool Black>
void OrderMoves(u8 movesSize, u8 depth) {
	for (u8 i = 0; i<21; i++) {
		moveIndexTempSize[i] = 0;
	}
	Move principalMove = NullMove;
	const BoardSet *rays = pinRays[firstOccupied(Black? boardState.black.king : boardState.white.king)];
	BoardSet kingRays = rays[0] | rays[1] | rays[2] | rays[3];
	for (u8 i = 0; i<movesSize; i++) {
		if (moves[i] == principleVariation.moves[principleVariation.cmove - depth]) {
			principalMove = moves[i];
			continue;
		}
		PositionValue value = simpleEvaluate<Black>(moves[i], kingRays);

		moveIndexTemp[value][moveIndexTempSize[value]] = i;
		moveIndexTempSize[value]++;
	}
	u8 index = 0;
	for (u8 i = 20; i >= 0; i--) {
		for (u8 j = 0; j<moveIndexTempSize[i]; j++) {
			movesTemp[index] = moves[moveIndexTemp[i][j]];
			index++;
		}
		if (i == 0) break;
	}
	if (principalMove.from == (u8) -1) {
		memcpy(moves.moves + moves.start, movesTemp, index * sizeof(Move));
	}
	else {
		memcpy(moves.moves + moves.start + 1, movesTemp, index * sizeof(Move));
		moves[0] = principalMove;
	}
}

template <bool Black>
PositionValue AlphaBeta(u8 depth, PositionValue alpha, PositionValue beta, Variation *principleVariation) {
	if (depth == 0) {
		principleVariation->cmove = 0;
		return Evaluate<Black>();
	}
	Variation variation = {};
	u8 movesSize = GenerateMoves<Black>();
	OrderMoves<Black>(movesSize, depth);
	if (movesSize == 0) {
		if (boardState.checkData.checkCount > 0) return MIN_VALUE - depth;
		else return DRAW_VALUE;
	}
	for (u8 i = 0; i<movesSize; i++) {
		moves[i].Make<Black>();
		moves.push(movesSize);
		PositionValue value = -AlphaBeta<!Black>(depth - 1, -beta, -alpha, &variation);
		moves.pop();
		moves[i].Unmake<Black>();
		if (value >= beta) {
			return beta;
		}
		if (value > alpha) {
			alpha = value;
			principleVariation->moves[0] = moves[i];
			memcpy(principleVariation->moves + 1, variation.moves, variation.cmove * sizeof(Move));
			principleVariation->cmove = variation.cmove + 1;
		}
		if (searchCanceled) return alpha;
	}
	return alpha;
}
