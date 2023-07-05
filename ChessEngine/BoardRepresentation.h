#pragma once
#include <stdint.h>
#include <intrin.h>
#include <string>

#define BoardSet uint64_t
#define u8 uint8_t
#define ChangeCaslingMask u8
#define CaslingState u8
#define Location u8
#define PieceType u8
#define File u8
#define Rank u8

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

	template<bool B>
	char PieceChar(PieceType type) {
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

bool bitAt(BoardSet set, Location loc);
bool isOccupied(BoardSet set, Location loc);
template <bool Bit>
void setBit(BoardSet *set, Location loc) {
	if constexpr (Bit) {
		*set |= 1ULL << loc;
	}
	else {
		*set &= ~(1ULL << loc);
	}
}
Location firstOccupied(BoardSet set);
Location lastOccupied(BoardSet set);
Location extractFirstOccupied(BoardSet *set);
u8 numberOfOccupancies(BoardSet set);

bool canWhiteCasleKingside(CaslingState state);
bool canWhiteCasleQueenside(CaslingState state);
bool canBlackCasleKingside(CaslingState state);
bool canBlackCasleQueenside(CaslingState state);

File fileOf(Location loc);
Rank rankOf(Location loc);

bool isColorBlack(PieceType type);
PieceType uncoloredType(PieceType type);

typedef struct BoardState {

	Location enPassantTarget;

	typedef struct CaslingStateHistory {
		CaslingState history[5];
		u8 stackPointer;

		CaslingState currentState();
		ChangeCaslingMask push(CaslingState newState);
		void pop();

		bool operator==(const CaslingStateHistory &) const = default;
	} CaslingStateHistory;

	CaslingStateHistory caslingStates;
	
	typedef struct PieceSets {
		BoardSet pawn;
		BoardSet knight;
		BoardSet bishop;
		BoardSet rook;
		BoardSet queen;
		BoardSet king;
		BoardSet all;

		bool operator==(const PieceSets &other) {
			if (pawn != other.pawn) return false;
			if (knight != other.knight) return false;
			if (bishop != other.bishop) return false;
			if (rook != other.rook) return false;
			if (queen != other.queen) return false;
			if (king != other.king) return false;
			if (all != other.all) return false;
			return true;
		}
	} PieceSets;

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

	typedef struct PinnedSets {
		BoardSet negativeDiagonal;
		BoardSet vertical;
		BoardSet positiveDiagonal;
		BoardSet horizontal;

		bool operator==(const PinnedSets &) const = default;
	} PinnedSets;

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

	bool operator==(const BoardState &other) {
		for (int i = 0; i<64; i++) if (squares[i] != other.squares[i]) return false;
		if (white != other.white) return false;
		if (black != other.black) return false;
		return true;
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


