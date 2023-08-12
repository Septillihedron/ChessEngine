#pragma once
#include <stdint.h>
#include <intrin.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include "Parameters.h"

#define BoardSet uint64_t
#define u8 uint8_t
#define u16 uint16_t
#define ChangeCaslingMask u8
#define CaslingState u8
#define Location u8
#define PieceType u8
#define File u8
#define Rank u8
#define FileAndCount u8

#include "BitOps.h"

constexpr Location NullLocation = 0b10000000;
constexpr CaslingState AllowAllCasling = 0b00001111;

constexpr BoardSet nullSet = 0ULL;
constexpr BoardSet allOccupiedSet = ~nullSet;

constexpr BoardSet AFile = 0x0101010101010101ULL;
constexpr BoardSet HFile = 0x8080808080808080ULL;
constexpr BoardSet secondRank = 0x000000000000FF00ULL;
constexpr BoardSet fourthRank = 0x00000000FF000000ULL;
constexpr BoardSet fifthRank = 0x000000FF00000000ULL;
constexpr BoardSet seventhRank = 0x00FF000000000000ULL;

// and no promotion
constexpr BoardSet oneMoveLocations = ~(secondRank | seventhRank);

template <bool Black> constexpr BoardSet twoMoveLocations = (Black)? seventhRank : secondRank;
template <bool Black> constexpr BoardSet promotionMoveLocations = (Black)? secondRank : seventhRank;
template <bool Black> constexpr BoardSet enPassantLocations = (Black)? fourthRank : fifthRank;

template <bool Black> constexpr Location forward = (Black)? (Location) -8 : (Location) 8;

namespace piece_type {
	constexpr PieceType NONE = 000;
	constexpr PieceType PAWN = 001;
	constexpr PieceType KNIGHT = 002;
	constexpr PieceType BISHOP = 003;
	constexpr PieceType ROOK = 004;
	constexpr PieceType QUEEN = 005;
	constexpr PieceType KING = 006;

	constexpr PieceType WHITE = 000;
	constexpr PieceType WHITE_PAWN = 001;
	constexpr PieceType WHITE_KNIGHT = 002;
	constexpr PieceType WHITE_BISHOP = 003;
	constexpr PieceType WHITE_ROOK = 004;
	constexpr PieceType WHITE_QUEEN = 005;
	constexpr PieceType WHITE_KING = 006;

	constexpr PieceType BLACK = 010;
	constexpr PieceType BLACK_PAWN = 011;
	constexpr PieceType BLACK_KNIGHT = 012;
	constexpr PieceType BLACK_BISHOP = 013;
	constexpr PieceType BLACK_ROOK = 014;
	constexpr PieceType BLACK_QUEEN = 015;
	constexpr PieceType BLACK_KING = 016;

	template <bool Black>
	constexpr PieceType COLOR = Black? BLACK : WHITE;
	template <bool Black = false>
	constexpr PieceType COLORED_PAWN = Black? BLACK_PAWN : WHITE_PAWN;
	template <bool Black = false>
	constexpr PieceType COLORED_KNIGHT = Black? BLACK_KNIGHT : WHITE_KNIGHT;
	template <bool Black = false>
	constexpr PieceType COLORED_BISHOP = Black? BLACK_BISHOP : WHITE_BISHOP;
	template <bool Black = false>
	constexpr PieceType COLORED_ROOK = Black? BLACK_ROOK : WHITE_ROOK;
	template <bool Black = false>
	constexpr PieceType COLORED_QUEEN = Black? BLACK_QUEEN : WHITE_QUEEN;
	template <bool Black = false>
	constexpr PieceType COLORED_KING = Black? BLACK_KING : WHITE_KING;

	inline char PieceChar(PieceType type) {
		switch (type) {
		case NONE:
			return '-';
		case WHITE_PAWN:
			return 'P';
		case WHITE_KNIGHT:
			return 'N';
		case WHITE_BISHOP:
			return 'B';
		case WHITE_ROOK:
			return 'R';
		case WHITE_QUEEN:
			return 'Q';
		case WHITE_KING:
			return 'K';
		case BLACK_PAWN:
			return 'p';
		case BLACK_KNIGHT:
			return 'n';
		case BLACK_BISHOP:
			return 'b';
		case BLACK_ROOK:
			return 'r';
		case BLACK_QUEEN:
			return 'q';
		case BLACK_KING:
			return 'k';
		default:
			throw "Not valid piece";
		}
	}
}

__forceinline
bool canWhiteCasleKingside(CaslingState state) {
	return state & 1;
}
__forceinline
bool canWhiteCasleQueenside(CaslingState state) {
	return (state >> 1) & 1;
}
__forceinline
bool canBlackCasleKingside(CaslingState state) {
	return (state >> 2) & 1;
}
__forceinline
bool canBlackCasleQueenside(CaslingState state) {
	return (state >> 3) & 1;
}

__forceinline
File fileOf(Location loc) {
	return loc & (u8) 7;
}
__forceinline
Rank rankOf(Location loc) {
	return loc >> (u8) 3;
}

__forceinline
bool isColorBlack(PieceType type) {
	return type >> 3;
}
__forceinline
PieceType uncoloredType(PieceType type) {
	return type & (u8) 7;
}

typedef struct State {
	union {
		struct {
			CaslingState casling;
			File enPassantFile;
			PieceType captured;
			u8 padding;
		};
		uint32_t state;
	};
	
	inline bool operator==(State &other) {
		return state == other.state;
	}
} State;

typedef struct PieceSets {
	union {
		struct {
			BoardSet none;
			BoardSet pawn;
			BoardSet knight;
			BoardSet bishop;
			BoardSet rook;
			BoardSet queen;
			BoardSet king;
			BoardSet all;
		};
		BoardSet pieces[8];
	};
	

	inline bool operator==(PieceSets &other) {
		if (pawn != other.pawn) return false;
		if (knight != other.knight) return false;
		if (bishop != other.bishop) return false;
		if (rook != other.rook) return false;
		if (queen != other.queen) return false;
		if (king != other.king) return false;
		if (all != other.all) return false;
		return true;
	}
	inline std::string Difference(const PieceSets &other) {
		std::stringstream difference;
		if (pawn != other.pawn) difference << "pawn, ";
		if (knight != other.knight) difference << "knight, ";
		if (bishop != other.bishop) difference << "bishop, ";
		if (rook != other.rook) difference << "rook, ";
		if (queen != other.queen) difference << "queen, ";
		if (king != other.king) difference << "king, ";
		if (all != other.all) difference << "all, ";
		return difference.str();
	}
} PieceSets;
typedef struct PinnedSets {
	BoardSet negativeDiagonal;
	BoardSet vertical;
	BoardSet positiveDiagonal;
	BoardSet horizontal;
	BoardSet diagonal;
	BoardSet lateral;
	BoardSet all;

	inline bool operator==(PinnedSets &other) {
		if (negativeDiagonal != other.negativeDiagonal) return false;
		if (vertical != other.vertical) return false;
		if (positiveDiagonal != other.positiveDiagonal) return false;
		if (horizontal != other.horizontal) return false;
		if (diagonal != other.diagonal) return false;
		if (lateral != other.lateral) return false;
		if (all != other.all) return false;
		return true;
	}
} PinnedSets;
typedef struct CheckData {
	u8 checkCount = 0;
	BoardSet checkRay = 0;
	BoardSet checkSource;

	inline bool operator==(CheckData &other) {
		if (checkCount != other.checkCount) return false;
		if (checkRay != other.checkRay) return false;
		return true;
	}
} CheckData;

typedef struct BoardState {

	State state;
	
	union {
		struct {
			PieceSets white;
			PieceSets black;
		};
		BoardSet pieces[16];
	};
	PieceSets whiteAttacks;
	PieceSets blackAttacks;
	PieceSets whiteDefends;
	PieceSets blackDefends;

	PieceType squares[64];

	PinnedSets whitePinnedSets;
	PinnedSets blackPinnedSets;
	CheckData checkData;

	__forceinline void ClearLocation(Location location, PieceSets &sets, BoardSet &set) {
		setBit<0>(&set, location);
		setBit<0>(&sets.all, location);
		squares[location] = piece_type::NONE;
	}
	template <PieceType type>
	__forceinline void SetLocation(Location location, PieceSets &sets, BoardSet &set) {
		setBit<1>(&set, location);
		setBit<1>(&sets.all, location);
		squares[location] = type;
	}
	template <bool ClearLocation>
	__forceinline
	void SetPiece(Location location, PieceType type) {
		PieceType originalType;
		if constexpr (ClearLocation) {
			originalType = squares[location];
		}
		else {
			originalType = type;
		}
		PieceSets &sets = isColorBlack(originalType)? black : white;
		BoardSet &set = sets.pieces[uncoloredType(originalType)];

		setBit<!ClearLocation>(&set, location);
		setBit<!ClearLocation>(&sets.all, location);

		if constexpr (ClearLocation) {
			squares[location] = piece_type::NONE;
		}
		else {
			squares[location] = type;
		}
	}


	template<bool Black>
	PinnedSets &GetPinnedSets() {
		if constexpr (Black) {
			return blackPinnedSets;
		}
		else {
			return whitePinnedSets;
		}
	}


	__inline void RemoveHistory() {
		
	}

	inline bool operator==(BoardState &other) {
		if (state != other.state) return false;
		if (white != other.white) return false;
		if (black != other.black) return false;
		for (int i = 0; i<64; i++) if (squares[i] != other.squares[i]) return false;
		return true;
	}

	inline std::string Difference(BoardState &other) {
		std::stringstream difference;
		if (state != other.state) difference << "state\n";
		for (Location i = 0; i<64; i++) {
			if (squares[i] != other.squares[i]) {
				difference << "square " << (char) (fileOf(i) + 'a') << (char) (rankOf(i) + '1') << "\n";
			}
		}
		if (white != other.white) difference << "white(" << white.Difference(other.white) << ")\n";
		if (black != other.black) difference << "black(" << black.Difference(other.black) << ")\n";
		return difference.str();
	}

	std::string GetStringRepresentation();

} BoardState;
bool CreateFromFEN(std::string fen, BoardState &boardState);

constexpr size_t a = sizeof(BoardState);
constexpr size_t b = 6*sizeof(PieceSets) + 64 + 2*sizeof(PinnedSets) + sizeof(CheckData);

extern BoardState boardState;

//template <bool Black> constexpr BoardSet pawnSet = (Black)? boardState.black.pawn : boardState.white.pawn;
//template <bool Black> constexpr BoardSet knightSet = (Black)? boardState.black.knight : boardState.white.knight;
//template <bool Black> constexpr BoardSet bishopSet = (Black)? boardState.black.bishop : boardState.white.bishop;
//template <bool Black> constexpr BoardSet rookSet = (Black)? boardState.black.rook : boardState.white.rook;
//template <bool Black> constexpr BoardSet queenSet = (Black)? boardState.black.queen : boardState.white.queen;
//
//template <bool Black> constexpr BoardSet friendlySet = (Black)? boardState.black.all : boardState.white.all;
//template <bool Black> constexpr BoardSet enemySet = (Black)? boardState.white.all : boardState.black.all;


