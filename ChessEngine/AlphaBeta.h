#pragma once
#include <stdint.h>
#include "Types.h"
#include "Parameters.h"
#include "MoveGen.h"

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
	std::cout << "Hash index collition: " << collitions0 << std::endl;
	std::cout << "Hash collition: " << collitions1 << std::endl;
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

inline PositionValue positionValues[256];
inline u8 moveTempSize[22];
inline Move moveTemp[22][256];

template <bool Black>
void OrderMoves(u8 movesSize, u8 depth) {
	for (u8 i = 0; i<22; i++) {
		moveTempSize[i] = 0;
	}

	if (movesSize > _Prefetch_Size_) {
		Hash prevHashes[_Prefetch_Size_];
		int prevHashesSP = 0;

		State prevState = boardState.state;
		Hash prevHash = boardState.hash;
		
		prefetchCaslingHash();
		for (int i = 0; i<_Prefetch_Size_; i++) {
			moves[i].MakeHashOnly<Black>();

			hashedPositions.prefetch(boardState.hash);
			prevHashes[i] = boardState.hash;

			moves[i].UnmakeHashOnly(prevState, prevHash);
		}
		for (int i = _Prefetch_Size_; i<movesSize; i++) {
			moves[i].MakeHashOnly<Black>();
			Hash hash = prevHashes[prevHashesSP];

			hashedPositions.prefetch(boardState.hash);
			prevHashes[prevHashesSP] = boardState.hash;
			prevHashesSP = (prevHashesSP + 1) & (_Prefetch_Size_ - 1);

			HashEntry entry = hashedPositions.get(hash);
			if (entry.hash != hash) {
				moveTemp[21][moveTempSize[21]] = moves[i-_Prefetch_Size_];
				moveTempSize[21]++;
			}
			else {
				moveTemp[0][moveTempSize[0]] = moves[i-_Prefetch_Size_];
				positionValues[moveTempSize[0]] = entry.value;
				moveTempSize[0]++;
			}
			moves[i].UnmakeHashOnly(prevState, prevHash);
		}
		for (int i = movesSize; i<movesSize+_Prefetch_Size_; i++) {
			Hash hash = prevHashes[prevHashesSP];

			prevHashesSP = (prevHashesSP + 1) & (_Prefetch_Size_ - 1);

			HashEntry entry = hashedPositions.get(hash);
			if (entry.hash != hash) {
				moveTemp[21][moveTempSize[21]] = moves[i-_Prefetch_Size_];
				moveTempSize[21]++;
			}
			else {
				moveTemp[0][moveTempSize[0]] = moves[i-_Prefetch_Size_];
				positionValues[moveTempSize[0]] = entry.value;
				moveTempSize[0]++;
			}
		}
	}
	else {
		State prevState = boardState.state;
		Hash prevHash = boardState.hash;
		for (int i = 0; i<movesSize; i++) {
			moves[i].MakeHashOnly<Black>();

			HashEntry entry = hashedPositions.get(boardState.hash);
			if (entry.hash != boardState.hash) {
				moveTemp[21][moveTempSize[21]] = moves[i];
				moveTempSize[21]++;
			}
			else {
				moveTemp[0][moveTempSize[0]] = moves[i];
				positionValues[moveTempSize[0]] = entry.value;
				moveTempSize[0]++;
			}
			moves[i].UnmakeHashOnly(prevState, prevHash);
		}
	}

	{ // wikipedia pseudocode implementation
		int i, j;
		i = 1;
		while (i < moveTempSize[0]) {
			j = i;
			while (j > 0 && positionValues[j-1] > positionValues[j]) {
				std::swap(positionValues[j], positionValues[j-1]);
				std::swap(moveTemp[0][j], moveTemp[0][j-1]);
				j = j - 1;
			}
			i = i + 1;
		}
	}
	memcpy(moves.moves + moves.start, moveTemp[0], moveTempSize[0] * sizeof(Move));
	u8 start = moveTempSize[0];
	moveTempSize[0] = 0;

	Move principalMove = NullMove;
	BoardSet &king = Black? boardState.black.king : boardState.white.king;
	BoardSet kingRays = RaysMoveSet<true, true>(king, boardState.black.all | boardState.white.all);
	for (u8 i = 0; i<moveTempSize[21]; i++) {
		if (moveTemp[21][i] == principleVariation.moves[principleVariation.cmove - depth]) {
			principalMove = moveTemp[21][i];
			continue;
		}
		PositionValue value = simpleEvaluate<Black>(moveTemp[21][i], kingRays);

		moveTemp[value][moveTempSize[value]] = moveTemp[21][i];
		moveTempSize[value]++;
	}
	if (principalMove.fromAndType != (u8) -1) {
		moves[start] = principalMove;
		start++;
	}
	for (int i = 20; i >= 0; i--) {
		memcpy(moves.moves + moves.start + start, moveTemp[i], moveTempSize[i] * sizeof(Move));
		start += moveTempSize[i];
	}
	if constexpr (_Strict_) {
		if (start != movesSize) {
			std::cout << "sorting went wrong" << std::endl;
			throw "sorting went wrong";
		}
	}
}

template <bool Black>
PositionValue AlphaBeta(u8 depth, PositionValue alpha, PositionValue beta, Variation *principleVariation) {
	Move *last12Moves = movesPlayed.data() + movesPlayed.size() - 12;
	if (last12Moves[11] == last12Moves[11-4]) {
		if (last12Moves[11-4] == last12Moves[11-4-4]) {
			principleVariation->cmove = 0;
			return DRAW_VALUE;
		}
	}
	if (depth == 0) {
		principleVariation->cmove = 0;
		PositionValue posVal = Evaluate<Black>();//AlphaBetaCaptures<!Black>(-beta, -alpha);
		hashedPositions.set(boardState.hash, posVal, depth);
		return posVal;
	}
	u8 movesSize = GenerateMoves<Black>();
	if (movesSize == 0) {
		principleVariation->cmove = 0;
		if (boardState.checkData.checkCount > 0) {
			hashedPositions.set(boardState.hash, MIN_VALUE, depth);
			return MIN_VALUE;
		}
		else {
			hashedPositions.set(boardState.hash, DRAW_VALUE, depth);
			return DRAW_VALUE;
		}
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
		if (searchCanceled) return alpha;
		if (value >= beta) {
			hashedPositions.set(boardState.hash, value, depth);
			return beta;
		}
		if (value > alpha) {
			alpha = value;
			principleVariation->moves[0] = moves[i];
			memcpy(principleVariation->moves + 1, variation.moves, variation.cmove * sizeof(Move));
			principleVariation->cmove = variation.cmove + 1;
			if (value == MAX_VALUE) {
				hashedPositions.set(boardState.hash, value, depth);
				return value;
			}
		}
	}
	hashedPositions.set(boardState.hash, alpha, depth);
	return alpha;
}
