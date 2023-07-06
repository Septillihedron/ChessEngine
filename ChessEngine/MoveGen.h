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

	void MakeMoveChangeCaslingRights();
	template <bool Black>
	void UpdateAttackAndDefendSets(PieceType pieceColoredType);
	template <bool Black>
	void Make();
	inline void Make(bool Black) {
		if (Black) Make<true>();
		else Make<false>();
	}
	template <bool Black>
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

extern int captures;

inline void Move::MakeMoveChangeCaslingRights() {
	if (from == 0 || to == 0) {
		CaslingState caslingState = boardState.caslingStates.currentState();
		caslingState &= ~0b0010;
		metadata |= boardState.caslingStates.push(caslingState);
	}
	else if (from == 7 || to == 7) {
		CaslingState caslingState = boardState.caslingStates.currentState();
		caslingState &= ~0b0001;
		metadata |= boardState.caslingStates.push(caslingState);
	}
	else if (from == 56 || to == 56) {
		CaslingState caslingState = boardState.caslingStates.currentState();
		caslingState &= ~0b1000;
		metadata |= boardState.caslingStates.push(caslingState);
	}
	else if (from == 63 || to == 63) {
		CaslingState caslingState = boardState.caslingStates.currentState();
		caslingState &= ~0b0100;
		metadata |= boardState.caslingStates.push(caslingState);
	}

	PieceType pieceType = boardState.squares[from];
	if (pieceType == piece_type::WHITE_KING) {
		CaslingState caslingState = boardState.caslingStates.currentState();
		caslingState &= ~0b0011;
		metadata |= boardState.caslingStates.push(caslingState);
	}
	else if (pieceType == piece_type::BLACK_KING) {
		CaslingState caslingState = boardState.caslingStates.currentState();
		caslingState &= ~0b1100;
		metadata |= boardState.caslingStates.push(caslingState);
	}
}

template <bool Black>
__forceinline
BoardSet pawnAttackSet(BoardSet pieceSet) {
	if constexpr (Black) {
		return ((pieceSet & ~AFile) >> (8-1)) | ((pieceSet & ~HFile) >> (8+1)); // left and right attacks
	}
	else {
		return ((pieceSet & ~AFile) << (8-1)) | ((pieceSet & ~HFile) << (8+1)); // left and right attacks
	}
	
}
template <bool Black>
__forceinline
BoardSet pawnMoveSet(BoardSet pieceSet) {
	BoardSet allPieces = boardState.white.all | boardState.black.all;
	constexpr BoardSet twoMoveSquares = twoMoveLocations<Black>;
	if constexpr (Black) {
		BoardSet singleStep = (pieceSet >> 8) & ~allPieces;
		BoardSet anotherMovePawns = singleStep & (twoMoveSquares >> 8);
		BoardSet doubleStep = (anotherMovePawns >> 8) & ~allPieces;
		return singleStep | doubleStep;
	}
	else {
		BoardSet singleStep = (pieceSet << 8) & ~allPieces;
		BoardSet anotherMovePawns = singleStep & (twoMoveSquares << 8);
		BoardSet doubleStep = (anotherMovePawns << 8) & ~allPieces;
		return singleStep | doubleStep;
	}
}
__forceinline
BoardSet knightMoveSet(BoardSet pieceSet) {
	BoardSet moves = 0;
	while (pieceSet != 0) {
		Location location = extractFirstOccupied(&pieceSet);
		moves |= knightMoves[location];
	}
	return moves;
}
template <size_t Ray_Index>
__forceinline
BoardSet __RayMoveSet(const BoardSet *rays, BoardSet highBits, BoardSet lowBits, BoardSet allPieces) {
	BoardSet highRayFirstPiece = onlyFirstBit(highBits & rays[Ray_Index] & allPieces);
	BoardSet lowRayLastPiece = onlyLastBit(lowBits & rays[Ray_Index] & allPieces);
	BoardSet highRay = ((highRayFirstPiece - 1) | highRayFirstPiece) & highBits & rays[Ray_Index];
	BoardSet lowRay = ~(lowRayLastPiece - 1) & lowBits & rays[Ray_Index];

	return highRay | lowRay;
}
template <bool Laterals, bool Diagonals>
__forceinline
BoardSet RaysMoveSet(BoardSet pieceSet) {
	BoardSet allPieces = boardState.black.all | boardState.white.all;
	BoardSet moves = 0;
	while (pieceSet != 0) {
		Location location = firstOccupied(pieceSet);
		const BoardSet *rays = pinRays[location];

		BoardSet queen = 1ULL << location;
		pieceSet &= ~queen;
		BoardSet lowBits = (queen - 1);
		BoardSet highBits = ~(lowBits | queen);

		if constexpr (Laterals) {
			moves |= __RayMoveSet<1>(rays, highBits, lowBits, allPieces);
			moves |= __RayMoveSet<3>(rays, highBits, lowBits, allPieces);
		}
		if constexpr (Diagonals) {
			moves |= __RayMoveSet<0>(rays, highBits, lowBits, allPieces);
			moves |= __RayMoveSet<2>(rays, highBits, lowBits, allPieces);
		}

	}
	return moves;
}
__forceinline
BoardSet kingMoveSet(BoardSet pieceSet) {
	return kingMoves[firstOccupied(pieceSet)];
}

template <bool Black>
void Move::UpdateAttackAndDefendSets(PieceType pieceColoredType) {
	PieceType pieceType = uncoloredType(pieceColoredType);

	PieceSets *pieceSets;
	PieceSets *attackSets;
	PieceSets *defendSets;
	if constexpr (Black) {
		pieceSets = &boardState.black;
		attackSets = &boardState.blackAttacks;
		defendSets = &boardState.blackDefends;
	}
	else {
		pieceSets = &boardState.white;
		attackSets = &boardState.whiteAttacks;
		defendSets = &boardState.whiteDefends;
	}
	switch (pieceType) {
	case piece_type::PAWN:
		attackSets->pawn = pawnAttackSet<Black>(pieceSets->pawn);
		defendSets->pawn = pawnMoveSet<Black>(pieceSets->pawn);
		break;
	case piece_type::KNIGHT:
		defendSets->knight = attackSets->knight = knightMoveSet(pieceSets->knight);
		break;
	case piece_type::BISHOP:
		defendSets->bishop = attackSets->bishop = RaysMoveSet<false, true>(pieceSets->bishop);
		break;
	case piece_type::ROOK:
		defendSets->rook = attackSets->rook = RaysMoveSet<true, false>(pieceSets->rook);
		break;
	case piece_type::QUEEN:
		defendSets->queen = attackSets->queen = RaysMoveSet<true, true>(pieceSets->queen);
		break;
	case piece_type::KING:
		attackSets->king |= kingMoveSet(pieceSets->king);
		break;
	default:
		throw std::invalid_argument("Invalid piece type");
	}
	attackSets->all = attackSets->pawn | attackSets->knight |attackSets->bishop | attackSets->rook | attackSets->queen | attackSets->king;
	defendSets->all = defendSets->pawn | defendSets->knight |defendSets->bishop | defendSets->rook | defendSets->queen | defendSets->king;
}

template <bool Black>
void Move::Make() {
	PieceType pieceType = boardState.squares[from];

	boardState.SetPiece<true>(from, piece_type::NONE);

	PieceType capturedPiece = boardState.squares[to];
	if (capturedPiece != piece_type::NONE) {
		captures++;
		metadata |= uncoloredType(capturedPiece) << 4;
		boardState.SetPiece<true>(to, piece_type::NONE);
		UpdateAttackAndDefendSets<!Black>(capturedPiece);
	}

	PieceType promotionPiece = PromotionPiece();
	if (promotionPiece != piece_type::NONE) {
		promotionPiece |= pieceType & 0b1000;
		boardState.SetPiece<false>(to, promotionPiece);
		UpdateAttackAndDefendSets<Black>(promotionPiece);
	}
	else {
		boardState.SetPiece<false>(to, pieceType);
	}

	UpdateAttackAndDefendSets<Black>(pieceType);

	MakeMoveChangeCaslingRights();
}
template <bool Black>
void Move::Unmake() {

	PieceType pieceType = boardState.squares[to];
	PieceType pieceColor = pieceType & 0b1000;

	PieceType capturedPiece = CapturedPiece();
	if (capturedPiece == piece_type::NONE) {
		boardState.SetPiece<true>(to, piece_type::NONE);
	}
	else {
		boardState.SetPiece<true>(to, piece_type::NONE);
		boardState.SetPiece<false>(to, capturedPiece | (~pieceColor & 0b1000));
		UpdateAttackAndDefendSets<!Black>(capturedPiece);
	}

	PieceType promotionPiece = PromotionPiece();
	if (promotionPiece != piece_type::NONE) {
		boardState.SetPiece<false>(from, piece_type::PAWN | pieceColor);
		UpdateAttackAndDefendSets<!Black>(promotionPiece);
		UpdateAttackAndDefendSets<!Black>(piece_type::PAWN);
	}
	else {
		boardState.SetPiece<false>(from, pieceType);
		UpdateAttackAndDefendSets<!Black>(pieceType);
	}
	if (ChangedCaslingRights()) {
		boardState.caslingStates.pop();
	}
}

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
	BoardSet pinnedSet;
	if constexpr (Black) {
		knightSet = boardState.black.knight;
		friendlyPieces = boardState.black.all;
		BoardSet *pinnedSets = boardState.blackPinnedSets.indexed;
		pinnedSet = (pinnedSets[0] | pinnedSets[1]) | (pinnedSets[2] | pinnedSets[3]);
	}
	else {
		knightSet = boardState.white.knight;
		friendlyPieces = boardState.white.all;
		BoardSet *pinnedSets = boardState.whitePinnedSets.indexed;
		pinnedSet = (pinnedSets[0] | pinnedSets[1]) | (pinnedSets[2] | pinnedSets[3]);
	}
	size_t numberOfMoves = 0;
	knightSet &= ~pinnedSet;
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

template <bool Black, bool In_En_Passant_Locations, bool In_Promotion_Locations, File Direction>
size_t generatePawnAttacks(size_t start, BoardSet pawnSet, BoardSet enemyPieces) {
	NamedPinnedSets *pinnedSets;

	if constexpr (Black) {
		pinnedSets = &boardState.blackPinnedSets.named;
	}
	else {
		pinnedSets = &boardState.whitePinnedSets.named;
	}
	if constexpr (Direction == (File) 1) {
		pawnSet &= ~pinnedSets->negativeDiagonal;
	}
	else {
		pawnSet &= ~pinnedSets->positiveDiagonal;
	}
	
	size_t numberOfMoves = 0;
	while (pawnSet != 0) {
		Location location = extractFirstOccupied(&pawnSet);
		Location target = location + forward<Black> + Direction;
		if constexpr (In_En_Passant_Locations) {
			if (boardState.enPassantTarget != NullLocation) {
				if (boardState.enPassantTarget == fileOf(location) + Direction) {
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

	size_t numberOfMoves = 0;
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, (File) -1>(start + numberOfMoves, pawnSet & ~AFile & specialMovesLocations, enemyPieces);
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, (File) +1>(start + numberOfMoves, pawnSet & ~HFile & specialMovesLocations, enemyPieces);
	return numberOfMoves;
}

// Recode all pawn moves and attacks (more bit hacks (shifting for enemy set))
template <bool Black>
size_t generatePawnMovesAndAttacks(size_t start) {
	BoardSet friendlyPieces;
	BoardSet enemyPieces;
	BoardSet allPieces;
	BoardSet pawnSet;
	NamedPinnedSets *pinnedSets;
	if constexpr (Black) {
		pawnSet = boardState.black.pawn;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
		pinnedSets = &boardState.blackPinnedSets.named;
	}
	else {
		pawnSet = boardState.white.pawn;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
		pinnedSets = &boardState.whitePinnedSets.named;
	}
	allPieces = friendlyPieces | enemyPieces;
	pawnSet &= ~pinnedSets->horizontal;
	size_t numberOfMoves = 0;
	// attacks
	BoardSet attackPawnSet = pawnSet & ~pinnedSets->vertical;
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, false, true>(start + numberOfMoves, attackPawnSet, enemyPieces); // promotions
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, true, true>(start + numberOfMoves, attackPawnSet, enemyPieces); // en passant
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, false, false>(start + numberOfMoves, attackPawnSet, enemyPieces); // normal
	// moves
	BoardSet movingPawnSet = pawnSet & ~(pinnedSets->negativeDiagonal | pinnedSets->positiveDiagonal);
	numberOfMoves += generatePawnMoves<Black, false, true>(start + numberOfMoves, movingPawnSet & promotionMoveLocations<Black>, allPieces); // promotions
	numberOfMoves += generatePawnMoves<Black, true, false>(start + numberOfMoves, movingPawnSet & twoMoveLocations<Black>, allPieces); // double steps
	numberOfMoves += generatePawnMoves<Black, false, false>(start + numberOfMoves, movingPawnSet & oneMoveLocations, allPieces); // single steps

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
	NamedPinnedSets *pinnedSets;
	if constexpr (Black) {
		rookSet = boardState.black.rook | boardState.black.queen;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
		pinnedSets = &boardState.blackPinnedSets.named;
	}
	else {
		rookSet = boardState.white.rook | boardState.white.queen;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
		pinnedSets = &boardState.whitePinnedSets.named;
	}
	BoardSet diagonallyPinned = pinnedSets->negativeDiagonal | pinnedSets->positiveDiagonal;
	rookSet &= ~diagonallyPinned;
	size_t numberOfMoves = 0;
	while (rookSet != 0) {
		Location location = extractFirstOccupied(&rookSet);//flipped
		if (!isOccupied(pinnedSets->horizontal, location)) {
			numberOfMoves += AddAllMovesInDirection<+1, +0>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
			numberOfMoves += AddAllMovesInDirection<-1, +0>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		}
		if (!isOccupied(pinnedSets->vertical, location)) {
			numberOfMoves += AddAllMovesInDirection<+0, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
			numberOfMoves += AddAllMovesInDirection<+0, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		}
	}
	return numberOfMoves;
}
template <bool Black>
size_t generateBishopLikeMoves(size_t start) {
	BoardSet friendlyPieces;
	BoardSet enemyPieces;
	BoardSet bishopSet;
	NamedPinnedSets *pinnedSets;
	if constexpr (Black) {
		bishopSet = boardState.black.bishop | boardState.black.queen;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
		pinnedSets = &boardState.blackPinnedSets.named;
	}
	else {
		bishopSet = boardState.white.bishop | boardState.white.queen;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
		pinnedSets = &boardState.whitePinnedSets.named;
	}
	BoardSet laterallyPinned = pinnedSets->vertical | pinnedSets->horizontal;
	bishopSet &= ~laterallyPinned;
	size_t numberOfMoves = 0;
	while (bishopSet != 0) {
		Location location = extractFirstOccupied(&bishopSet);
		if (!isOccupied(pinnedSets->positiveDiagonal, location)) {
			numberOfMoves += AddAllMovesInDirection<-1, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
			numberOfMoves += AddAllMovesInDirection<+1, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		}
		if (!isOccupied(pinnedSets->negativeDiagonal, location)) {
			numberOfMoves += AddAllMovesInDirection<+1, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
			numberOfMoves += AddAllMovesInDirection<-1, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		}
	}
	return numberOfMoves;
}

template <bool After_King>
__forceinline
BoardSet ScanPinRay(BoardSet attackingPieces, BoardSet enemyBlockingPieces, BoardSet friendlyBlockingPieces, BoardSet ray) {
	attackingPieces &= ray;
	if (attackingPieces == 0) return 0;
	// get the region between king and attacking piece
	if constexpr (After_King) {
		attackingPieces = onlyFirstBit(attackingPieces);
		ray &= (attackingPieces-1);
	}
	else {
		attackingPieces = onlyLastBit(attackingPieces);
		ray &= ~((attackingPieces - 1) | attackingPieces);
	}

	if ((enemyBlockingPieces & ray) != 0) return 0;
	friendlyBlockingPieces &= ray;
	if (numberOfOccupancies(friendlyBlockingPieces) != 1) return 0;
	return friendlyBlockingPieces;
}

template <bool Black>
void GeneratePinnedSets() {
	PinnedSets &pinnedSets = boardState.GetPinnedSets<Black>();

	BoardSet enemyRookLike;
	BoardSet enemyBishopLike;
	BoardSet enemyNonRookLike;
	BoardSet enemyNonBishopLike;
	BoardSet friendlyPieces;
	BoardSet kingSet;
	
	if constexpr (Black) {
		enemyRookLike = boardState.white.rook | boardState.white.queen;
		enemyBishopLike = boardState.white.bishop | boardState.white.queen;
		enemyNonRookLike = boardState.white.all & ~enemyRookLike;
		enemyNonBishopLike = boardState.white.all & ~enemyBishopLike;
		friendlyPieces = boardState.black.all;
		kingSet = boardState.black.king;
	}
	else {
		enemyRookLike = boardState.black.rook | boardState.black.queen;
		enemyBishopLike = boardState.black.bishop | boardState.black.queen;
		enemyNonRookLike = boardState.black.all & ~enemyRookLike;
		enemyNonBishopLike = boardState.black.all & ~enemyBishopLike;
		friendlyPieces = boardState.white.all;
		kingSet = boardState.white.king;
	}
	Location kingLocation = firstOccupied(kingSet);

	const BoardSet *pinRay = pinRays[kingLocation];

	BoardSet beforeKing = kingSet - 1;
	BoardSet afterKing = allOccupiedSet & ~(beforeKing | kingSet);

	pinnedSets.indexed[0] |= ScanPinRay<false>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[0] & beforeKing);
	pinnedSets.indexed[0] |= ScanPinRay<true>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[0] & afterKing);
	pinnedSets.indexed[2] |= ScanPinRay<false>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[2] & beforeKing);
	pinnedSets.indexed[2] |= ScanPinRay<true>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[2] & afterKing);

	pinnedSets.indexed[1] |= ScanPinRay<false>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[1] & beforeKing);
	pinnedSets.indexed[1] |= ScanPinRay<true>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[1] & afterKing);
	pinnedSets.indexed[3] |= ScanPinRay<false>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[3] & beforeKing);
	pinnedSets.indexed[3] |= ScanPinRay<true>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[3] & afterKing);

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

