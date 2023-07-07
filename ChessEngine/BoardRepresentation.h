#pragma once
#include <stdint.h>
#include <intrin.h>
#include <stdexcept>
#include <string>
#include <sstream>

#define BoardSet uint64_t
#define u8 uint8_t
#define ChangeCaslingMask u8
#define CaslingState u8
#define Location u8
#define PieceType u8
#define File u8
#define Rank u8

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

constexpr BoardSet oneMoveLocations = ~(secondRank | seventhRank); // and no promotion

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

	constexpr PieceType WHITE_PAWN = 001;
	constexpr PieceType WHITE_KNIGHT = 002;
	constexpr PieceType WHITE_BISHOP = 003;
	constexpr PieceType WHITE_ROOK = 004;
	constexpr PieceType WHITE_QUEEN = 005;
	constexpr PieceType WHITE_KING = 006;

	constexpr PieceType BLACK_PAWN = 011;
	constexpr PieceType BLACK_KNIGHT = 012;
	constexpr PieceType BLACK_BISHOP = 013;
	constexpr PieceType BLACK_ROOK = 014;
	constexpr PieceType BLACK_QUEEN = 015;
	constexpr PieceType BLACK_KING = 016;

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
	return loc & 7;
}
__forceinline
Rank rankOf(Location loc) {
	return loc >> 3;
}

__forceinline
bool isColorBlack(PieceType type) {
	return type >> 3;
}
__forceinline
PieceType uncoloredType(PieceType type) {
	return type & 7;
}

typedef struct CaslingStateHistory {
	CaslingState history[5];
	u8 stackPointer;

	CaslingState currentState();
	ChangeCaslingMask push(CaslingState newState);
	void pop();

	bool operator==(const CaslingStateHistory &) const = default;
} CaslingStateHistory;

typedef struct PieceSets {
	BoardSet pawn;
	BoardSet knight;
	BoardSet bishop;
	BoardSet rook;
	BoardSet queen;
	BoardSet king;
	BoardSet all;

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
typedef struct NamedPinnedSets {
	BoardSet negativeDiagonal;
	BoardSet vertical;
	BoardSet positiveDiagonal;
	BoardSet horizontal;
	BoardSet diagonal;
	BoardSet lateral;
	BoardSet all;

	inline bool operator==(NamedPinnedSets &other) {
		if (negativeDiagonal != negativeDiagonal) return false;
		if (vertical != vertical) return false;
		if (positiveDiagonal != positiveDiagonal) return false;
		if (horizontal != horizontal) return false;
		if (diagonal != diagonal) return false;
		if (lateral != lateral) return false;
		if (all != all) return false;
		return true;
	}
} NamedPinnedSets;
typedef union PinnedSets {
	BoardSet indexed[7];
	struct NamedPinnedSets named;

	inline bool operator==(PinnedSets &other) {
		return named == other.named;
	}
} PinnedSets;
typedef struct CheckData {
	u8 checkCount;
	BoardSet checkRay;
	BoardSet checkSource;

	inline bool operator==(CheckData &other) {
		if (checkCount != checkCount) return false;
		if (checkRay != checkRay) return false;
		return true;
	}
} CheckData;

typedef struct BoardState {

	Location enPassantTarget;

	CaslingStateHistory caslingStates;

	PieceSets white;
	PieceSets black;
	PieceSets whiteAttacks;
	PieceSets blackAttacks;
	PieceSets whiteDefends;
	PieceSets blackDefends;

	BoardSet &GetPieceSet(PieceType type);

	PieceType squares[64];

	template <bool ClearLocation>
	void SetPiece(Location location, PieceType type) {
		PieceType originalType;
		if constexpr (ClearLocation) {
			originalType = squares[location];
		}
		else {
			originalType = type;
		}
		PieceSets &sets = isColorBlack(originalType)? black : white;
		BoardSet &set = GetPieceSet(originalType);

		setBit<!ClearLocation>(&set, location);
		setBit<!ClearLocation>(&sets.all, location);

		if constexpr (ClearLocation) {
			squares[location] = piece_type::NONE;
		}
		else {
			squares[location] = type;
		}
	}

	PinnedSets whitePinnedSets;
	PinnedSets blackPinnedSets;

	template<bool Black>
	PinnedSets &GetPinnedSets() {
		if constexpr (Black) {
			return blackPinnedSets;
		}
		else {
			return whitePinnedSets;
		}
	}

	CheckData checkData;

	inline bool operator==(BoardState &other) {
		if (enPassantTarget != other.enPassantTarget) return false;
		if (caslingStates != caslingStates) return false;
		if (white != white) return false;
		if (black != black) return false;
		if (whiteAttacks != whiteAttacks) return false;
		if (blackAttacks != blackAttacks) return false;
		if (whiteDefends != whiteDefends) return false;
		if (blackDefends != blackDefends) return false;
		for (int i = 0; i<64; i++) if (squares[i] != other.squares[i]) return false;
		if (whitePinnedSets != whitePinnedSets) return false;
		if (blackPinnedSets != blackPinnedSets) return false;
		if (checkData != checkData) return false;
		return true;
	}

	inline std::string Difference(BoardState &other) {
		std::stringstream difference;
		for (int i = 0; i<64; i++) {
			if (squares[i] != other.squares[i]) {
				difference << "square(" << fileOf(i) + 'A' << ", " << (int) rankOf(i) + 1 << ")\n";
			}
		}
		if (white != other.white) difference << "white(" << white.Difference(other.white) << ")\n";
		if (black != other.black) difference << "black(" << black.Difference(other.black) << ")\n";
		return difference.str();
	}

	std::string GetStringRepresentation();

} BoardState;


extern BoardState boardState;

//template <bool Black> constexpr BoardSet pawnSet = (Black)? boardState.black.pawn : boardState.white.pawn;
//template <bool Black> constexpr BoardSet knightSet = (Black)? boardState.black.knight : boardState.white.knight;
//template <bool Black> constexpr BoardSet bishopSet = (Black)? boardState.black.bishop : boardState.white.bishop;
//template <bool Black> constexpr BoardSet rookSet = (Black)? boardState.black.rook : boardState.white.rook;
//template <bool Black> constexpr BoardSet queenSet = (Black)? boardState.black.queen : boardState.white.queen;
//
//template <bool Black> constexpr BoardSet friendlySet = (Black)? boardState.black.all : boardState.white.all;
//template <bool Black> constexpr BoardSet enemySet = (Black)? boardState.white.all : boardState.black.all;


