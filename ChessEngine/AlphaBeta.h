#pragma once
#include <stdint.h>

#include "MoveGen.h"

#define PositionValue int16_t
constexpr PositionValue MIN_VALUE = INT16_MIN+1;
constexpr PositionValue MAX_VALUE = INT16_MAX;
constexpr PositionValue DRAW_VALUE = 0;
constexpr Move NullMove = { -1, -1, -1 };

#include "Evaluator.h"

typedef struct Variation {
	u8 cmove;
	Move moves[10];
} Variation;

inline u8 maxDepth;
inline Variation principleVariation;
inline bool searchCanceled = false;

template <bool Black>
Move Search() {
	Variation principleVariationNext;
	Move bestMove;
	AlphaBetaRoot<Black>(1, &principleVariationNext);
	principleVariation = principleVariationNext;
	bestMove = principleVariation.moves[0];
	for (u8 i = 2; i < 6; i++) {
		AlphaBetaRoot<Black>(i, &principleVariationNext);
		if (searchCanceled) {
			principleVariation.cmove -= 2;
			memcpy(principleVariation.moves, principleVariation.moves + 2, principleVariation.cmove);
			return bestMove;
		}
		principleVariation = principleVariationNext;
		bestMove = principleVariation.moves[0];
	}
	principleVariation.cmove -= 2;
	memcpy(principleVariation.moves, principleVariation.moves + 2, principleVariation.cmove);
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

inline u8 moveIndexTemp[21][256];
inline Move movesTemp[256];

template <bool Black>
void OrderMoves(u8 movesSize, u8 depth) {
	for (u8 i = 0; i<21; i++) {
		moveIndexTemp[i][0] = 0;
	}
	Move principalMove = NullMove;
	const BoardSet *rays = pinRays[firstOccupied(Black? boardState.black.king : boardState.white.king)];
	BoardSet kingRays = rays[0] | rays[1] | rays[2] | rays[3];
	for (u8 i = 0; i<movesSize; i++) {
		if (moves[i] == principleVariation.moves[maxDepth - depth]) {
			principalMove = moves[i];
			continue;
		}
		PositionValue value = simpleEvaluate<Black>(moves[i], kingRays);

		moveIndexTemp[value][0]++;
		moveIndexTemp[value][moveIndexTemp[value][0]] = i;
	}
	u8 index = 0;
	for (u8 i = 20; i >= 0; i--) {
		for (u8 j = 1; j<moveIndexTemp[i][0] + 1; j++) {
			movesTemp[index] = moves[moveIndexTemp[i][j]];
			index++;
		}
		if (i == 0) break;
	}
	if (principalMove.from == (u8) -1) {
		memcpy(moves.moves + moves.start, movesTemp, sizeof(Move)*index);
	}
	else {
		memcpy(moves.moves + moves.start + 1, movesTemp, sizeof(Move)*index);
		moves[0] = principalMove;
	}
}

template <bool Black>
void AlphaBetaRoot(u8 depth, Variation *principleVariation) {
	PositionValue alpha = MIN_VALUE, beta = MAX_VALUE;
	u8 movesSize = GenerateMoves<Black>();
	//OrderMoves<Black>(movesSize, depth);
	if (movesSize == 0) {
		principleVariation->moves[0] = NullMove;
		return;
	}
	Variation variation;
	for (u8 i = 0; i<movesSize; i++) {
		moves[i].Make<Black>();
		moves.push(movesSize);
		PositionValue value = - AlphaBeta<!Black>(depth - 1, -beta, -alpha, &variation);
		moves.pop(movesSize);
		moves[i].Unmake<Black>();
		if (value > alpha) {
			alpha = value;
			principleVariation->moves[0] = moves[i];
			memcpy(principleVariation->moves + 1, variation.moves, variation.cmove * sizeof(Move));
			principleVariation->cmove = variation.cmove + 1;
		}
		if (searchCanceled) return;
	}
	std::cout << "depth " << (int) depth << ": " << std::endl
		<< "  value: " << (int) alpha << std::endl
		<< "  best move: " << principleVariation->moves[0].ToString() << std::endl;
}

template <bool Black>
PositionValue AlphaBeta(u8 depth, PositionValue alpha, PositionValue beta, Variation *principleVariation) {
	if (depth == 0) {
		principleVariation->cmove = 0;
		return Evaluate(Black);
	}
	Variation variation;
	u8 movesSize = GenerateMoves<Black>();
	//OrderMoves<Black>(movesSize, depth);
	if (movesSize == 0) {
		if (boardState.checkData.checkCount > 0) return Black? MAX_VALUE : MIN_VALUE;
		else return DRAW_VALUE;
	}
	for (u8 i = 0; i<movesSize; i++) {
		moves[i].Make<Black>();
		moves.push(movesSize);
		PositionValue value = -AlphaBeta<!Black>(depth - 1, - beta, - alpha, &variation);
		moves.pop(movesSize);
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
