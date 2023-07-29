#pragma once
#include "AlphaBeta.h"

const PositionValue pieceValues[] = {
	0, 1, 3, 3, 5, 9
};

template <bool Black>
PositionValue simpleEvaluate(Move move, BoardSet kingRays) {
	PositionValue pieceValue = pieceValues[boardState.squares[move.from] & 7];

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

constexpr BoardSet controlPositionWeightsSet[5] = {
	0xC3810000000081C3,
	0x3C4281818181423C,
	0x003C664242663C00,
	0x0000182424180000,
	0x0000001818000000,
};

__forceinline 
PositionValue EvaluatePieceControl(BoardSet attacks) {
	constexpr BoardSet ones = controlPositionWeightsSet[0] | controlPositionWeightsSet[2] | controlPositionWeightsSet[4];
	constexpr BoardSet twos = controlPositionWeightsSet[1] | controlPositionWeightsSet[2];
	constexpr BoardSet fours = controlPositionWeightsSet[3] | controlPositionWeightsSet[4];
	return
		1*numberOfOccupancies(attacks & ones) +
		2*numberOfOccupancies(attacks & twos) +
		4*numberOfOccupancies(attacks & fours);
}

__forceinline
PositionValue EvaluateControl() {
	PositionValue whitePawns = EvaluatePieceControl(boardState.whiteAttacks.pawn);
	PositionValue whiteKnights = EvaluatePieceControl(boardState.whiteAttacks.knight);
	PositionValue whiteBishops = EvaluatePieceControl(boardState.whiteAttacks.bishop);
	PositionValue whiteRooks = EvaluatePieceControl(boardState.whiteAttacks.rook);
	PositionValue whiteQueens = EvaluatePieceControl(boardState.whiteAttacks.queen);
	PositionValue whiteKings = EvaluatePieceControl(boardState.whiteAttacks.king);

	PositionValue blackPawns = EvaluatePieceControl(boardState.blackAttacks.pawn);
	PositionValue blackKnights = EvaluatePieceControl(boardState.blackAttacks.knight);
	PositionValue blackBishops = EvaluatePieceControl(boardState.blackAttacks.bishop);
	PositionValue blackRooks = EvaluatePieceControl(boardState.blackAttacks.rook);
	PositionValue blackQueens = EvaluatePieceControl(boardState.blackAttacks.queen);
	PositionValue blackKings = EvaluatePieceControl(boardState.blackAttacks.king);

	PositionValue pawns = (whitePawns - blackPawns);
	PositionValue knights = (whiteKnights - blackKnights);
	PositionValue bishops = (whiteBishops - blackBishops);
	PositionValue rooks = (whiteRooks - blackRooks);
	PositionValue queens = (whiteQueens - blackQueens);
	PositionValue kings = (whiteKings - blackKings);

	return
		+5 * pawns +
		+4 * knights +
		+4 * bishops +
		+2 * rooks +
		+1 * queens +
		-1 * kings;
}

template <bool Black>
inline PositionValue Evaluate() {
	UpdateAttackAndDefendSets<true>();
	UpdateAttackAndDefendSets<false>();

	PositionValue material = EvaluateMaterial();
	PositionValue control = EvaluateControl();
	PositionValue turn = 1;
	PositionValue total = DRAW_VALUE + material + control + turn;
	return Black? -total : total;
}

