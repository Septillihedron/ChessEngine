#pragma once

#include <cstdlib>
#include <stdint.h>
#include "BoardRepresentation.h"
#include "MoveSets.h"

// 0CCCRPPP
// C = captured piece
// R = changed casling rights
// P = promotion piece
#define MoveMetadata u8

typedef struct Move {
	Location from;
	Location to;
	MoveMetadata metadata;

	PieceType PromotionPiece();
	bool ChangedCaslingRights();
	PieceType CapturedPiece();

	void Make();
	void Unmake();
} Move;

typedef struct MovesArray {
	size_t capacity;
	size_t start;
	Move *moves;

	MovesArray();

	void push(size_t size);
	void pop(size_t size);
	void resizeAdd();
	void resizeSub();

	Move &operator[](size_t index);

} MovesArray;

extern MovesArray moves;

inline void AddAllMoves(size_t start, BoardSet moveSet, Move move) {
	for (size_t i = 0; moveSet != 0; i++) {
		Location location = extractFirstOccupied(&moveSet);
		move.to = location;
		moves[start + i] = move;
	}
}

template <bool Black>
size_t generateKingMoves(size_t start) {
	Location location;
	BoardSet invalidLocations;
	if constexpr (Black) {
		location = firstOccupied(boardState.black.king);
		invalidLocations = boardState.black.all | boardState.whiteAttacks.all;
	}
	else {
		location = firstOccupied(boardState.white.king);
		invalidLocations = boardState.white.all | boardState.blackAttacks.all;
	}
	BoardSet kingMoveLocations = kingMoves[location] & ~invalidLocations;
	AddAllMoves(start, kingMoveLocations, { location, 0, piece_type::NONE });
	return numberOfOccupancies(kingMoveLocations);
}

template <bool Black>
size_t generateKnightMoves(size_t start) {
	BoardSet friendlyPieces;
	BoardSet knightSet;
	if constexpr (Black) {
		knightSet = boardState.black.knight;
		friendlyPieces = boardState.black.all;
	}
	else {
		knightSet = boardState.white.knight;
		friendlyPieces = boardState.white.all;
	}
	size_t numberOfMoves = 0;
	while (knightSet != 0) {
		Location location = extractFirstOccupied(&knightSet);
		BoardSet knightMoveLocations = knightMoves[location] & ~friendlyPieces;
		AddAllMoves(start + numberOfMoves, knightMoveLocations, { location, 0, piece_type::NONE });
		numberOfMoves += numberOfOccupancies(knightMoveLocations);
	}
	return numberOfMoves;
}

template <bool In_Promotion_Locations>
size_t generatePawnMovesWithPossiblePromotion(size_t start, Location from, Location to) {
	if constexpr (In_Promotion_Locations) {
		moves[start + 0] = { from, to, piece_type::KNIGHT };
		moves[start + 1] = { from, to, piece_type::BISHOP };
		moves[start + 2] = { from, to, piece_type::ROOK };
		moves[start + 3] = { from, to, piece_type::QUEEN };
		return 4;
	}
	else {
		moves[start + 0] = { from, to, piece_type::NONE };
		return 1;
	}
}

template <bool Black, bool In_Two_Move_Locations, bool In_Promotion_Locations>
size_t generatePawnMoves(size_t start, BoardSet pawnSet, BoardSet allPieces) {
	size_t numberOfMoves = 0;
	while (pawnSet != 0) {
		Location location = extractFirstOccupied(&pawnSet);
		Location target = location + forward<Black>;
		if (!isOccupied(allPieces, target)) {
			numberOfMoves += generatePawnMovesWithPossiblePromotion<In_Promotion_Locations>(start + numberOfMoves, location, target);
			if constexpr (In_Two_Move_Locations) {
				target += forward<Black>;
				if (!isOccupied(allPieces, target)) {
					moves[start + numberOfMoves] = { location, target, piece_type::NONE };
					numberOfMoves++;
				}
			}
		}
	}
	return numberOfMoves;
}

template <bool Black, bool In_En_Passant_Locations, bool In_Promotion_Locations, bool In_A_File, bool In_H_File>
size_t generatePawnAttacks(size_t start, BoardSet pawnSet, BoardSet enemyPieces) {
	size_t numberOfMoves = 0;
	while (pawnSet != 0) {
		Location location = extractFirstOccupied(&pawnSet);
		if constexpr (!In_A_File) {
			Location target = location + forward<Black> - 1; // forward left from white's prespective
			if constexpr(In_En_Passant_Locations) {
				if (boardState.enPassantTarget != NullLocation) {
					if (boardState.enPassantTarget == fileOf(location) - 1) { // if it can en passant to the left from white's prespective
						moves[start + numberOfMoves] = { location, target, piece_type::NONE };
						numberOfMoves++;
					}
				}
			}
			else {
				if (isOccupied(enemyPieces, target)) {
					numberOfMoves += generatePawnMovesWithPossiblePromotion<In_Promotion_Locations>(start + numberOfMoves, location, target);
				}
			}
		}

		if constexpr (!In_H_File) {
			Location target = location + forward<Black> + 1; // forward right from white's prespective
			if constexpr (In_En_Passant_Locations) {
				if (boardState.enPassantTarget != NullLocation) {
					if (boardState.enPassantTarget == fileOf(location) + 1) { // if it can en passant to the right from white's prespective
						moves[start + numberOfMoves] = { location, target, piece_type::NONE };
						numberOfMoves++;
					}
				}
			}
			else {
				if (isOccupied(enemyPieces, target)) {
					numberOfMoves += generatePawnMovesWithPossiblePromotion<In_Promotion_Locations>(start + numberOfMoves, location, target);
				}
			}
		}
	}
	return numberOfMoves;
}

template <bool Black, bool In_En_Passant_Locations, bool In_Promotion_Locations>
constexpr BoardSet specialMoveLocations() {
	if constexpr (In_En_Passant_Locations) {
		return enPassantLocations<Black>;
	}
	if constexpr (In_Promotion_Locations) {
		return promotionMoveLocations<Black>;
	}
	return allOccupiedSet;
}

template <bool Black, bool In_En_Passant_Locations, bool In_Promotion_Locations>
size_t generatePawnAttacksWithPossibleSpecialMoves(size_t start, BoardSet pawnSet, BoardSet enemyPieces) {

	constexpr BoardSet specialMovesLocations = specialMoveLocations<Black, In_En_Passant_Locations, In_Promotion_Locations>();
	constexpr BoardSet centerPawns = specialMovesLocations & ~(AFile | HFile);
	constexpr BoardSet AFilePawns = AFile & specialMovesLocations;
	constexpr BoardSet HFilePawns = HFile & specialMovesLocations;

	size_t numberOfMoves = 0;
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, false, false>(start + numberOfMoves, pawnSet & centerPawns, enemyPieces);
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, true, false>(start + numberOfMoves, pawnSet & AFilePawns, enemyPieces);
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, false, true>(start + numberOfMoves, pawnSet & HFilePawns, enemyPieces);
	return numberOfMoves;
}

template <bool Black>
size_t generatePawnMovesAndAttacks(size_t start) {
	BoardSet friendlyPieces;
	BoardSet enemyPieces;
	BoardSet allPieces;
	BoardSet pawnSet;
	if constexpr (Black) {
		pawnSet = boardState.black.pawn;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
	}
	else {
		pawnSet = boardState.white.pawn;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
	}
	allPieces = friendlyPieces | enemyPieces;
	size_t numberOfMoves = 0;
	// attacks
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, false, true>(start + numberOfMoves, pawnSet, enemyPieces); // promotions
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, true, true>(start + numberOfMoves, pawnSet, enemyPieces); // en passant
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, false, false>(start + numberOfMoves, pawnSet, enemyPieces); // normal
	// moves
	numberOfMoves += generatePawnMoves<Black, false, true>(start + numberOfMoves, pawnSet & promotionMoveLocations<Black>, allPieces); // promotions
	numberOfMoves += generatePawnMoves<Black, true, false>(start + numberOfMoves, pawnSet & twoMoveLocations<Black>, allPieces); // double steps
	numberOfMoves += generatePawnMoves<Black, false, false>(start + numberOfMoves, pawnSet & oneMoveLocations, allPieces); // single steps

	return numberOfMoves;
}

template <Location File_Direction, Location Rank_Direction>
bool IsOnEdge(Location location) {
	if constexpr (File_Direction == (Location) -1) {
		if (fileOf(location) == 0) return true;
	}
	else if constexpr (File_Direction == (Location) 1) {
		if (fileOf(location) == 7) return true;
	}
	if constexpr (Rank_Direction == (Location) -1) {
		if (rankOf(location) == 0) return true;
	}
	else if constexpr (Rank_Direction == (Location) 1) {
		if (rankOf(location) == 7) return true;
	}
	return false;
}

template <Location File_Direction, Location Rank_Direction>
size_t AddAllMovesInDirection(size_t start, Location location, BoardSet enemyPieces, BoardSet friendlyPieces) {
	constexpr Location direction = (Location) (8*Rank_Direction + File_Direction);

	Location target = location;
	size_t numberOfMoves = 0;
	while (!IsOnEdge<File_Direction, Rank_Direction>(target)) {
		target += direction;

		if (isOccupied(friendlyPieces, target)) break;

		moves[start + numberOfMoves] = { location, target, piece_type::NONE };
		numberOfMoves++;

		if (isOccupied(enemyPieces, target)) break;
	}
	return numberOfMoves;
}

// optimize to perfect hash function
template <bool Black>
size_t generateRookLikeMoves(size_t start) {
	BoardSet friendlyPieces;
	BoardSet enemyPieces;
	BoardSet rookSet;
	if constexpr (Black) {
		rookSet = boardState.black.rook | boardState.black.queen;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
	}
	else {
		rookSet = boardState.white.rook | boardState.white.queen;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
	}

	size_t numberOfMoves = 0;
	while (rookSet != 0) {
		Location location = extractFirstOccupied(&rookSet);
		
		numberOfMoves += AddAllMovesInDirection<+1, +0>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		numberOfMoves += AddAllMovesInDirection<-1, +0>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		numberOfMoves += AddAllMovesInDirection<+0, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		numberOfMoves += AddAllMovesInDirection<+0, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
	}
	return numberOfMoves;
}
template <bool Black>
size_t generateBishopLikeMoves(size_t start) {
	BoardSet friendlyPieces;
	BoardSet enemyPieces;
	BoardSet bishopSet;
	if constexpr (Black) {
		bishopSet = boardState.black.bishop | boardState.black.queen;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
	}
	else {
		bishopSet = boardState.white.bishop | boardState.white.queen;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
	}

	size_t numberOfMoves = 0;
	while (bishopSet != 0) {
		Location location = extractFirstOccupied(&bishopSet);

		numberOfMoves += AddAllMovesInDirection<+1, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		numberOfMoves += AddAllMovesInDirection<+1, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		numberOfMoves += AddAllMovesInDirection<-1, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		numberOfMoves += AddAllMovesInDirection<-1, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
	}
	return numberOfMoves;
}

template <bool Black>
void GeneratePinnedSets() {
	BoardState::PinnedSets &pinnedSets = boardState.GetPinnedSets<Black>();

	BoardSet enemyRookLike;
	BoardSet enemyBishopLike;
	BoardSet enemyNonRookLike;
	BoardSet enemyNonBishopLike;
	BoardSet friendlyPieces;

	if constexpr (Black) {
		enemyRookLike = boardState.white.rook | boardState.white.queen;
		enemyBishopLike = boardState.white.bishop | boardState.white.queen;
		enemyNonRookLike = boardState.white.all & ~enemyRookLike;
		enemyNonBishopLike = boardState.white.all & ~enemyBishopLike;
		friendlyPieces = boardState.black.all;
	}
	else {
		enemyRookLike = boardState.black.rook | boardState.black.queen;
		enemyBishopLike = boardState.black.bishop | boardState.black.queen;
		enemyNonRookLike = boardState.black.all & ~enemyRookLike;
		enemyNonBishopLike = boardState.black.all & ~enemyBishopLike;
		friendlyPieces = boardState.white.all;
	}



}

template <bool Black>
size_t GenerateMoves() {
	if (moves.start + 350 > moves.capacity) {
		moves.resizeAdd();
	}
	GeneratePinnedSets<Black>();
	size_t movesSize = 0;
	movesSize += generateKingMoves<Black>(movesSize);
	movesSize += generateKnightMoves<Black>(movesSize);
	movesSize += generatePawnMovesAndAttacks<Black>(movesSize);
	movesSize += generateRookLikeMoves<Black>(movesSize);
	movesSize += generateBishopLikeMoves<Black>(movesSize);
	return movesSize;
}

