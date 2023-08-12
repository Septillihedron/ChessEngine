#pragma once
#include <stdint.h>
#include "Parameters.h"
#include "MoveGen.h"

#define PositionValue int16_t
constexpr PositionValue MAX_VALUE = INT16_MAX-1;
constexpr PositionValue MIN_VALUE = -MAX_VALUE;
constexpr PositionValue DRAW_VALUE = 0;
constexpr Move NullMove = { 0xffff };

#include "Evaluator.h"

inline u8 searchDepth = 0;

typedef struct Variation {
	u8 cmove;
	Move moves[_Max_Search_Depth_];

	Variation() : cmove(0) {}
} Variation;

inline Variation variations[_Max_Search_Depth_];

inline Variation principleVariation;
inline bool searchCanceled = false;

template <bool Black>
Move Search() {
	Variation principleVariationNext = {};
	Move bestMove;
	AlphaBeta<Black>(1, MIN_VALUE-1, MAX_VALUE+1, &principleVariationNext);
	principleVariation = principleVariationNext;
	bestMove = principleVariation.moves[0];
	for (u8 i = 2; i < 7; i++) {
		searchDepth = i;
		PositionValue value = AlphaBeta<Black>(i, MIN_VALUE-1, MAX_VALUE+1, &principleVariationNext);
		std::string betsMoveStr = principleVariationNext.moves[0].ToString();
		std::cout << "depth " << (int) i << ": " << (int) value << " ";

		std::cout << "(";
		for (int i = 0; i<principleVariationNext.cmove; i++) {
			std::cout << principleVariationNext.moves[i].ToString() << " ";
		}
		std::cout << ")" << std::endl;

		if (searchCanceled) {
			break;
		}
		principleVariation = principleVariationNext;
		bestMove = principleVariation.moves[0];
		if (value == MAX_VALUE || value == MIN_VALUE) break;
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

inline u8 moveTempSize[21];
inline Move moveTemp[21][256];

template <bool Black>
void OrderMoves(u8 movesSize, u8 depth) {
	for (u8 i = 0; i<21; i++) {
		moveTempSize[i] = 0;
	}
	Move principalMove = NullMove;
	BoardSet &king = Black? boardState.black.king : boardState.white.king;
	BoardSet kingRays = RaysMoveSet<true, true>(king, boardState.black.all | boardState.white.all);
	for (u8 i = 0; i<movesSize; i++) {
		if (moves[i] == principleVariation.moves[principleVariation.cmove - depth]) {
			principalMove = moves[i];
			continue;
		}
		PositionValue value = simpleEvaluate<Black>(moves[i], kingRays);

		moveTemp[value][moveTempSize[value]] = moves[i];
		moveTempSize[value]++;
	}
	u8 start = (principalMove.fromAndType == (u8) -1)? 0 : 1;
	if (principalMove.fromAndType != (u8) -1) {
		moves[0] = principalMove;
		start++;
	}
	for (int i = 20; i >= 0; i--) {
		memcpy(moves.moves + moves.start + start, moveTemp[i], moveTempSize[i] * sizeof(Move));
		start += moveTempSize[i];
	}
}

template <bool Black>
PositionValue AlphaBeta(u8 depth, PositionValue alpha, PositionValue beta, Variation *principleVariation) {
	if (depth == 0) {
		principleVariation->cmove = 0;
		return Evaluate<Black>();
	}
	Move *last12Moves = movesPlayed.data() + movesPlayed.size() - 12;
	if (last12Moves[11] == last12Moves[11-4]) {
		if (last12Moves[11-4] == last12Moves[11-4-4]) {
			principleVariation->cmove = 0;
			return DRAW_VALUE;
		}
	}
	u8 movesSize = GenerateMoves<Black>();
	if (movesSize == 0) {
		principleVariation->cmove = 0;
		if (boardState.checkData.checkCount > 0) return MIN_VALUE;
		else return DRAW_VALUE;
	}
	Variation &variation = variations[depth];
	variation.cmove = 0;
	OrderMoves<Black>(movesSize, depth);
	State prevState = boardState.state;
	for (u8 i = 0; i<movesSize; i++) {
		moves[i].Make<Black>();
		moves.push(movesSize);
		PositionValue value = -AlphaBeta<!Black>(depth - 1, -beta, -alpha, &variation);
		moves.pop();
		moves[i].Unmake<Black>(prevState);
		if (value >= beta) {
			return beta;
		}
		if (value > alpha) {
			alpha = value;
			principleVariation->moves[0] = moves[i];
			memcpy(principleVariation->moves + 1, variation.moves, variation.cmove * sizeof(Move));
			principleVariation->cmove = variation.cmove + 1;
			if (value == MAX_VALUE) {
				return value;
			}
		}
		if (searchCanceled) return alpha;
	}
	return alpha;
}
