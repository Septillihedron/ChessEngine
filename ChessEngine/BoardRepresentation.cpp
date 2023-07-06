
#include <cstring>
#include <stdexcept>
#include "BoardRepresentation.h"

BoardState CreateBoardState() {
	BoardState boardState = { NullLocation, { AllowAllCasling }, 0 };
	boardState.white =
	{
		0b11111111ULL << 8,
		0b01000010ULL << 0,
		0b00100100ULL << 0,
		0b10000001ULL << 0,
		0b00001000ULL << 0,
		0b00010000ULL << 0,
		0b1111111111111111ULL << 0,
	};
	boardState.black =
	{
		0b11111111ULL << 48,
		0b01000010ULL << 56,
		0b00100100ULL << 56,
		0b10000001ULL << 56,
		0b00001000ULL << 56,
		0b00010000ULL << 56,
		0b1111111111111111ULL << 48,
	};
	PieceType squares[] = {
		004, 002, 003, 005, 006, 003, 002, 004,
		001, 001, 001, 001, 001, 001, 001, 001,
		000, 000, 000, 000, 000, 000, 000, 000,
		000, 000, 000, 000, 000, 000, 000, 000,
		000, 000, 000, 000, 000, 000, 000, 000,
		000, 000, 000, 000, 000, 000, 000, 000,
		011, 011, 011, 011, 011, 011, 011, 011,
		014, 012, 013, 015, 016, 013, 012, 014,
	};
	memcpy(boardState.squares, squares, 64);
	return boardState;
}

CaslingState CaslingStateHistory::currentState() {
	return history[stackPointer];
}

ChangeCaslingMask CaslingStateHistory::push(CaslingState newState) {
	if (newState != currentState()) {
		stackPointer++;
		history[stackPointer] = newState;
		return 0b1000;
	}
	return 0b0000;
}
void CaslingStateHistory::pop() {
	stackPointer--;
}

BoardSet &BoardState::GetPieceSet(PieceType type) {
	PieceSets &sets = isColorBlack(type)? black : white;
	switch (uncoloredType(type)) {
	case piece_type::PAWN:
		return sets.pawn;
	case piece_type::KNIGHT:
		return sets.knight;
	case piece_type::BISHOP:
		return sets.bishop;
	case piece_type::ROOK:
		return sets.rook;
	case piece_type::QUEEN:
		return sets.queen;
	case piece_type::KING:
		return sets.king;
	default:
		throw std::invalid_argument("Invalid piece type");
	}
}

std::string BoardState::GetStringRepresentation() {
	std::string s = "";
	for (int y = 7; y>=0; y--) {
		for (int x = 0; x<8; x++) {
			s += piece_type::PieceChar<0>(squares[y*8 + x]);
		}
		s += "\n";
	}
	return s;
}

BoardState boardState = CreateBoardState();
