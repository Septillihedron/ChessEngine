
#include <cstring>
#include <stdexcept>
#include "BoardRepresentation.h"
#include "MoveGen.h"

BoardState CreateBoardState() {

	//BoardState boardState = {
	//	Location enPassantTarget;

	//	CaslingStateHistory caslingStates;

	//	PieceSets white;
	//	PieceSets black;
	//	PieceSets whiteAttacks;
	//	PieceSets blackAttacks;
	//	PieceSets whiteDefends;
	//	PieceSets blackDefends;

	//	PieceType squares[64];

	//	PinnedSets whitePinnedSets;
	//	PinnedSets blackPinnedSets;

	//	CheckData checkData;
	//}

	BoardState boardState = { { { NullLocation }, (u8) 0 }, { { AllowAllCasling }, 0 }, 0 };
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

PieceType CharToPieceType(char c) {
	bool isBlack = c > 'a';
	PieceType colorMask = isBlack? piece_type::BLACK : piece_type::WHITE;
	switch (isBlack? c-('a'-'A') : c)
	{
	case 'P':
		return piece_type::PAWN | colorMask;
	case 'N':
		return piece_type::KNIGHT | colorMask;
	case 'B':
		return piece_type::BISHOP | colorMask;
	case 'R':
		return piece_type::ROOK | colorMask;
	case 'Q':
		return piece_type::QUEEN | colorMask;
	case 'K':
		return piece_type::KING | colorMask;
	default:
		throw std::invalid_argument("Invalid piece type");
	}
}

bool CreateFromFEN(std::string fen, BoardState &boardState) {
	EnPassantHistory enPassantTargets = { { NullLocation }, (u8) 0 };
	CaslingStateHistory caslingStates = { { 0 }, 0 };
	for (Location i = 0; i<64; i++) {
		if (boardState.squares[i] != piece_type::NONE) {
			boardState.SetPiece<true>(i, piece_type::NONE);
		}
	}
	bool isBlackStart = 0;
	int state = 0;
	int rank = 7;
	int file = 0;
	for (int i = 0; i<fen.size(); i++) {
		char c = fen[i];
		if (c == ' ') {
			state++;
		}
		if (state == 0) {
			if (c == '/') {
				file = 0;
				rank--;
				continue;
			}
			if (c >= '1' && c <= '9') {
				file += c - '0';
				continue;
			}
			boardState.SetPiece<false>(rank*8 + file, CharToPieceType(c));
			file++;
		}
		else if (state == 1) {
			if (c == 'w') isBlackStart = 0;
			else if (c == 'b') isBlackStart = 1;
		}
		else if (state == 2) {
			if (c == '-') continue;
			if (c == 'K') caslingStates.history[0] |= 0b0001;
			if (c == 'Q') caslingStates.history[0] |= 0b0010;
			if (c == 'k') caslingStates.history[0] |= 0b0100;
			if (c == 'q') caslingStates.history[0] |= 0b1000;
		}
	}
	boardState.enPassantTargets = enPassantTargets;
	boardState.caslingStates = caslingStates;
	UpdateAttackAndDefendSets<false>();
	UpdateAttackAndDefendSets<true>();
	return isBlackStart;
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
Location EnPassantHistory::current() {
	if (stackPointer >= 33) throw std::invalid_argument("Invalid state");
	return history[stackPointer];
}
u8 EnPassantHistory::push(Location newLocation) {
	if (newLocation != current()) {
		stackPointer++;
		history[stackPointer] = newLocation;
		return 0b10000000;
	}
	return 0b00000000;
}
void EnPassantHistory::pop() {
	stackPointer--;
}
void CaptureStack::push(PieceType type) {
	stackPointer++;
	stack[stackPointer] = type;
}
PieceType CaptureStack::pop() {
	PieceType type = stack[stackPointer];
	stackPointer--;
	return type;
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
			s += piece_type::PieceChar(squares[y*8 + x]);
		}
		s += "\n";
	}
	return s;
}

BoardState boardState = CreateBoardState();
