
#include <cstring>
#include <stdexcept>
#include "BoardRepresentation.h"
#include "MoveGen.h"

BoardState CreateBoardState() {

	BoardState boardState = { };
	boardState.state = { AllowAllCasling, NullLocation, piece_type::NONE };
	boardState.white =
	{
		0,
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
		0,
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

	bool isBlackStart = false;

	boardState.hash = 0;
	boardState.hash ^= isBlackStart? colorHash : 0;
	boardState.hash ^= caslingHashes[boardState.state.casling];
	for (Location i = 0; i<64; i++) {
		PieceType type = boardState.squares[i];
		if (type == piece_type::NONE) continue;
		type -= (type & 0b1000)? piece_type::BLACK_PAWN : piece_type::WHITE_PAWN;
		boardState.hash ^= pieceHashes[type][i];
	}
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
	for (Location i = 0; i<64; i++) {
		if (boardState.squares[i] != piece_type::NONE) {
			boardState.SetPiece<true>(i, piece_type::NONE);
		}
	}
	boardState.state.casling = 0b0000;
	bool isBlackStart = 0;
	int state = 0;
	int rank = 7;
	int file = 0;
	for (size_t i = 0; i<fen.size(); i++) {
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
			if (c == 'K') boardState.state.casling |= 0b0001;
			if (c == 'Q') boardState.state.casling |= 0b0010;
			if (c == 'k') boardState.state.casling |= 0b0100;
			if (c == 'q') boardState.state.casling |= 0b1000;
		}
	}
	UpdateAttackAndDefendSets<false>();
	UpdateAttackAndDefendSets<true>();

	boardState.hash = 0;
	boardState.hash ^= isBlackStart? colorHash : 0;
	boardState.hash ^= caslingHashes[boardState.state.casling];
	for (Location i = 0; i<64; i++) {
		PieceType type = boardState.squares[i];
		if (type == piece_type::NONE) continue;
		type -= (type & 0b1000)? piece_type::BLACK_PAWN : piece_type::WHITE_PAWN;
		boardState.hash ^= pieceHashes[type][i];
	}

	return isBlackStart;
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
