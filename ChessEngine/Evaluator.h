#pragma once
#include "AlphaBeta.h"

const PositionValue pieceValues[] = {
	0, 1, 3, 3, 5, 9
};

template <bool Black>
PositionValue simpleEvaluate(Move move, BoardSet kingRays) {
	PositionValue pieceValue = pieceValues[boardState.squares[move.from] & 7];

	BoardSet enemies = Black? boardState.white.all : boardState.black.all;
	BoardSet enemyAttacks = Black? boardState.whiteAttacks.all : boardState.blackAttacks.all;
	BoardSet defends = Black? boardState.blackDefends.all : boardState.blackDefends.all;

	PositionValue value = 9;

	PositionValue capturedValue = pieceValues[boardState.squares[move.to] & 7];
	value += capturedValue;
	if (isOccupied(enemyAttacks, move.to)) value -= pieceValue;
	if (isOccupied(defends, move.to)) value++;
	if (isOccupied(kingRays, move.to)) value++;
	return value; // min 0, max 21
}

__forceinline
PositionValue EvaluateMaterial() {
	PositionValue blackMaterial =
		100*numberOfOccupancies(boardState.black.pawn) +
		300*numberOfOccupancies(boardState.black.knight) +
		302*numberOfOccupancies(boardState.black.bishop) +
		500*numberOfOccupancies(boardState.black.rook) +
		900*numberOfOccupancies(boardState.black.queen);
	PositionValue whiteMaterial =
		100*numberOfOccupancies(boardState.white.pawn) +
		300*numberOfOccupancies(boardState.white.knight) +
		302*numberOfOccupancies(boardState.white.bishop) +
		500*numberOfOccupancies(boardState.white.rook) +
		900*numberOfOccupancies(boardState.white.queen);
	return whiteMaterial - blackMaterial;
}

inline const PositionValue controlPositionWeights[] = {
	1, 1, 2, 2, 2, 2, 1, 1,
	1, 2, 3, 3, 3, 3, 2, 1,
	2, 3, 3, 4, 4, 3, 3, 2,
	2, 3, 4, 5, 5, 4, 3, 2,
	2, 3, 4, 5, 5, 4, 3, 2,
	2, 3, 3, 4, 4, 3, 3, 2,
	1, 2, 3, 3, 3, 3, 2, 1,
	1, 1, 2, 2, 2, 2, 1, 1,
};

__forceinline 
PositionValue EvaluatePieceControl(BoardSet attacks) {
	PositionValue control = 0;
	while (attacks != 0) {
		control += controlPositionWeights[extractFirstOccupied(&attacks)];
	}
	return control;
}

__forceinline
PositionValue EvaluateControl() {
	PositionValue blackControl =
		3 * EvaluatePieceControl(boardState.blackAttacks.pawn) +
		2 * EvaluatePieceControl(boardState.blackAttacks.knight) +
		2 * EvaluatePieceControl(boardState.blackAttacks.bishop) +
		1 * EvaluatePieceControl(boardState.blackAttacks.rook) +
		1 * EvaluatePieceControl(boardState.blackAttacks.queen) -
		1 * EvaluatePieceControl(boardState.blackAttacks.king);

	PositionValue whiteControl = 
		3 * EvaluatePieceControl(boardState.whiteAttacks.pawn) +
		2 * EvaluatePieceControl(boardState.whiteAttacks.knight) +
		2 * EvaluatePieceControl(boardState.whiteAttacks.bishop) +
		1 * EvaluatePieceControl(boardState.whiteAttacks.rook) +
		1 * EvaluatePieceControl(boardState.whiteAttacks.queen) -
		1 * EvaluatePieceControl(boardState.whiteAttacks.king);
	return whiteControl - blackControl;
}

inline PositionValue Evaluate(bool isBlackTurn) {
	PositionValue material = EvaluateMaterial();
	PositionValue control = EvaluateControl();
	return DRAW_VALUE + material + control + (isBlackTurn? -1 : 1);
}

