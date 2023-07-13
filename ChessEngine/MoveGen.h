#pragma once

#include <cstdlib>
#include <stdint.h>
#include <format>
#include "BoardRepresentation.h"
#include "MoveSets.h"

// ECCCRPPP
// EC = EnPassant capture
// E  = Changed EnPassant target
// C  = captured piece
// R  = changed casling rights
// P  = promotion piece
#define MoveMetadata u8

constexpr MoveMetadata EnPassantCapture = 0b111;

typedef struct Move {
	Location from;
	Location to;
	MoveMetadata metadata;

	__forceinline PieceType PromotionPiece() {
		return metadata & 7;
	}
	__forceinline bool ChangedCaslingRights() {
		return (metadata >> 3) & 1;
	}
	__forceinline PieceType CapturedPiece() {
		return (metadata >> 4) & 7;
	}
	__forceinline bool ChangedEnPassantTarget() {
		return metadata >> 7;
	}

	void MakeMoveChangeCaslingRights(PieceType pieceType);
	template <bool Black, bool Unmake>
	void MakeMoveMoveCaslingRook();
	template <bool Black>
	void Make();
	inline void Make(bool Black) {
		if (Black) Make<true>();
		else Make<false>();
	}
	template <bool Black>
	void Unmake();

	inline char PromotionPiece(PieceType type) {
		switch (type)
		{
		case piece_type::KNIGHT:
			return 'n';
		case piece_type::BISHOP:
			return 'b';
		case piece_type::ROOK:
			return 'r';
		case piece_type::QUEEN:
			return 'q';
		default:
			return '-';
			break;
		}
	}

	inline std::string ToString() {
		u8 fromFile = fileOf(from);
		u8 fromRank = rankOf(from);
		u8 toFile = fileOf(to);
		u8 toRank = rankOf(to);

		if ((metadata & 7) == 0) {
			return std::format("{:c}{:c}{:c}{:c}", fromFile + 'a', fromRank + '1', toFile + 'a', toRank + '1');
		}
		else {
			char promotionPiece = PromotionPiece(metadata & 7);
			return std::format("{:c}{:c}{:c}{:c}{:c}", fromFile + 'a', fromRank + '1', toFile + 'a', toRank + '1', promotionPiece);
		}
	}

	inline bool operator==(Move &other) {
		if (from != other.from) return false;
		if (to != other.to) return false;
		if ((metadata & 7) != (other.metadata & 7)) return false;
		return true;
	}
} Move;

typedef struct MovesArray {
	u16 capacity;
	u16 start;
	Move *moves;

	MovesArray();

	__forceinline void push(u16 size) {
		start += size;
	}
	__forceinline void pop(u16 size) {
		start -= size;
	}
	__forceinline void resizeAdd() {
		capacity += 350;
		moves = (Move *) realloc(moves, capacity * sizeof Move);
	}
	__forceinline void resizeSub() {
		capacity -= 350;
		moves = (Move *) realloc(moves, capacity * sizeof Move);
	}

	__forceinline Move &operator[](u8 index) {
		return this->moves[this->start + index];
	}

} MovesArray;

extern MovesArray moves;

extern int captures;

inline void Move::MakeMoveChangeCaslingRights(PieceType pieceType) {
	CaslingState caslingState = boardState.caslingStates.currentState();
	if (from == 0 || to == 0) {
		caslingState &= ~0b0010;
	}
	if (from == 7 || to == 7) {
		caslingState &= ~0b0001;
	}
	if (from == 56 || to == 56) {
		caslingState &= ~0b1000;
	}
	if (from == 63 || to == 63) {
		caslingState &= ~0b0100;
	}

	if (pieceType == piece_type::WHITE_KING) {
		caslingState &= ~0b0011;
	}
	else if (pieceType == piece_type::BLACK_KING) {
		caslingState &= ~0b1100;
	}
	metadata |= boardState.caslingStates.push(caslingState);
}

template <bool Black, bool Reversed>
void Move::MakeMoveMoveCaslingRook() {
	CaslingState caslingState = boardState.caslingStates.currentState();
	if constexpr (Black) {
		if (canBlackCasleKingside(caslingState)) {
			if (from == 074 && to == 076) {
				boardState.SetPiece<true>(Reversed? 075 : 077, piece_type::NONE);
				boardState.SetPiece<false>(Reversed? 077 : 075, piece_type::BLACK_ROOK);
			}
		}
		if (canBlackCasleQueenside(caslingState)) {
			if (from == 074 && to == 072) {
				boardState.SetPiece<true>(Reversed? 073 : 070, piece_type::NONE);
				boardState.SetPiece<false>(Reversed? 070 : 073, piece_type::BLACK_ROOK);
			}
		}
	}
	else {
		if (canWhiteCasleKingside(caslingState)) {
			if (from == 004 && to == 006) {
				boardState.SetPiece<true>(Reversed? 005 : 007, piece_type::NONE);
				boardState.SetPiece<false>(Reversed? 007 : 005, piece_type::WHITE_ROOK);
			}
		}
		if (canWhiteCasleQueenside(caslingState)) {
			if (from == 004 && to == 002) {
				boardState.SetPiece<true>(Reversed? 003 : 000, piece_type::NONE);
				boardState.SetPiece<false>(Reversed? 000 : 003, piece_type::WHITE_ROOK);
			}
		}
	}
}


template <bool Black>
__forceinline
BoardSet pawnAttackSet(BoardSet pieceSet) {
	if constexpr (Black) {
		return ((pieceSet & ~HFile) >> (8-1)) | ((pieceSet & ~AFile) >> (8+1)); // left and right attacks
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
	BoardSet highRay = ((highRayFirstPiece != 0)? ((highRayFirstPiece - 1) | highRayFirstPiece) : allOccupiedSet) & highBits & rays[Ray_Index];
	BoardSet lowRay = ((lowRayLastPiece != 0)? ~(lowRayLastPiece - 1) : allOccupiedSet) & lowBits & rays[Ray_Index];

	return highRay | lowRay;
}
template <bool Laterals, bool Diagonals>
__forceinline
BoardSet RaysMoveSet(BoardSet pieceSet, BoardSet blockingPieces) {
	BoardSet moves = 0;
	while (pieceSet != 0) {
		Location location = firstOccupied(pieceSet);
		const BoardSet *rays = pinRays[location];

		BoardSet piece = 1ULL << location;
		pieceSet &= ~piece;
		BoardSet lowBits = (piece - 1);
		BoardSet highBits = ~(lowBits | piece);

		if constexpr (Laterals) {
			moves |= __RayMoveSet<1>(rays, highBits, lowBits, blockingPieces);
			moves |= __RayMoveSet<3>(rays, highBits, lowBits, blockingPieces);
		}
		if constexpr (Diagonals) {
			moves |= __RayMoveSet<0>(rays, highBits, lowBits, blockingPieces);
			moves |= __RayMoveSet<2>(rays, highBits, lowBits, blockingPieces);
		}

	}
	return moves;
}
__forceinline
BoardSet kingMoveSet(BoardSet pieceSet) {
	return kingMoves[firstOccupied(pieceSet)];
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

	if ((enemyBlockingPieces & ray) != 0) return 0; // if an enemy piece is blocking the check
	friendlyBlockingPieces &= ray;
	u8 numberOfBlockers = numberOfOccupancies(friendlyBlockingPieces);
	if (numberOfBlockers == 0) {
		boardState.checkData.checkCount++;
		boardState.checkData.checkRay = ray;
		boardState.checkData.checkSource = attackingPieces;
		return 0;
	}
	else if (numberOfBlockers == 1) {
		return friendlyBlockingPieces;
	}
	else {
		return 0;
	}
}
template <bool Black>
void UpdateAttackAndDefendSets() {
	PieceSets *pieceSets;
	PieceSets *attackSets;
	PieceSets *defendSets;
	BoardSet enemyKing;
	if constexpr (Black) {
		pieceSets = &boardState.black;
		attackSets = &boardState.blackAttacks;
		defendSets = &boardState.blackDefends;
		enemyKing = boardState.white.king;
	}
	else {
		pieceSets = &boardState.white;
		attackSets = &boardState.whiteAttacks;
		defendSets = &boardState.whiteDefends;
		enemyKing = boardState.black.king;
	}
	attackSets->pawn = pawnAttackSet<Black>(pieceSets->pawn);
	defendSets->pawn = pawnMoveSet<Black>(pieceSets->pawn);
	defendSets->knight = attackSets->knight = knightMoveSet(pieceSets->knight);
	BoardSet blockingPieces = (boardState.black.all | boardState.white.all) & ~enemyKing;
	defendSets->bishop = attackSets->bishop = RaysMoveSet<false, true>(pieceSets->bishop, blockingPieces);
	defendSets->rook = attackSets->rook = RaysMoveSet<true, false>(pieceSets->rook, blockingPieces);
	defendSets->queen = attackSets->queen = RaysMoveSet<true, true>(pieceSets->queen, blockingPieces);
	defendSets->king = attackSets->king = kingMoveSet(pieceSets->king);

	attackSets->all = attackSets->pawn | attackSets->knight | attackSets->bishop | attackSets->rook | attackSets->queen | attackSets->king;
	defendSets->all = defendSets->pawn | defendSets->knight | defendSets->bishop | defendSets->rook | defendSets->queen | defendSets->king;
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

	boardState.checkData.checkCount = 0;

	pinnedSets.named.negativeDiagonal = ScanPinRay<false>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[0] & beforeKing);
	pinnedSets.named.negativeDiagonal |= ScanPinRay<true>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[0] & afterKing);
	pinnedSets.named.positiveDiagonal = ScanPinRay<false>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[2] & beforeKing);
	pinnedSets.named.positiveDiagonal |= ScanPinRay<true>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[2] & afterKing);

	pinnedSets.named.vertical = ScanPinRay<false>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[1] & beforeKing);
	pinnedSets.named.vertical |= ScanPinRay<true>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[1] & afterKing);
	pinnedSets.named.horizontal = ScanPinRay<false>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[3] & beforeKing);
	pinnedSets.named.horizontal |= ScanPinRay<true>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[3] & afterKing);

	pinnedSets.named.diagonal = pinnedSets.named.negativeDiagonal | pinnedSets.named.positiveDiagonal;
	pinnedSets.named.lateral = pinnedSets.named.vertical | pinnedSets.named.horizontal;

	pinnedSets.named.all = pinnedSets.named.diagonal | pinnedSets.named.lateral;
}
template <bool Black>
void CheckUncheckedChecks() {
	BoardSet king;
	BoardSet knights;
	BoardSet pawns;
	if constexpr (Black) {
		knights = boardState.white.knight;
		pawns = boardState.white.pawn;
		king = boardState.black.king;
	}
	else {
		knights = boardState.black.knight;
		pawns = boardState.black.pawn;
		king = boardState.white.king;
	}
	/* knight checks */ {
		Location kingLocation = firstOccupied(king);
		BoardSet checkingKnights = knightMoves[kingLocation] & knights;
		if (checkingKnights != 0) {
			boardState.checkData.checkCount++;
			boardState.checkData.checkRay = checkingKnights;
			boardState.checkData.checkSource = checkingKnights;
		}
	}
	/* pawn checks */ {
		BoardSet checkingPawns;
		if constexpr (Black) {
			checkingPawns = (((king & ~HFile) >> 7) | ((king & ~AFile) >> 9)) & pawns;
		}
		else {
			checkingPawns = (((king & ~AFile) << 7) | ((king & ~HFile) << 9)) & pawns;
		}
		if (checkingPawns != 0) {
			boardState.checkData.checkCount++;
			boardState.checkData.checkRay = checkingPawns;
			boardState.checkData.checkSource = checkingPawns;
		}
	}
}

template <bool Black>
void Move::Make() {
	PieceType pieceType = boardState.squares[from];

	boardState.SetPiece<true>(from, piece_type::NONE);

	PieceType capturedPiece = boardState.squares[to];
	if (uncoloredType(pieceType) == piece_type::PAWN && boardState.enPassantTargets.current() == fileOf(to) && isOccupied(enPassantLocations<Black>, from)) {
		metadata |= EnPassantCapture << 4;
		boardState.SetPiece<true>(to - forward<Black>, piece_type::NONE);
	}
	else if (capturedPiece != piece_type::NONE) {
		captures++;
		metadata |= uncoloredType(capturedPiece) << 4;
		boardState.SetPiece<true>(to, piece_type::NONE);
	}

	PieceType promotionPiece = PromotionPiece();
	if (promotionPiece != piece_type::NONE) {
		promotionPiece |= pieceType & 0b1000;
		boardState.SetPiece<false>(to, promotionPiece);
	}
	else {
		boardState.SetPiece<false>(to, pieceType);
	}
	File enPassantTarget = NullLocation;
	if (uncoloredType(pieceType) == piece_type::PAWN) {
		Rank rankDifference = rankOf(to) - rankOf(from);
		if (rankDifference == ((u8) -2) || rankDifference == 2) {
			enPassantTarget = fileOf(to);
		}
	}
	u8 mask = boardState.enPassantTargets.push(enPassantTarget);
	metadata = metadata | mask;

	MakeMoveMoveCaslingRook<Black, false>();

	MakeMoveChangeCaslingRights(pieceType);

	UpdateAttackAndDefendSets<Black>();
	UpdateAttackAndDefendSets<!Black>();
}
template <bool Black>
void Move::Unmake() {

	PieceType pieceType = boardState.squares[to];
	PieceType pieceColor = pieceType & 0b1000;

	PieceType capturedPiece = CapturedPiece();
	if (capturedPiece == EnPassantCapture) {
		if constexpr (Black) {
			boardState.SetPiece<false>(to - forward<Black>, piece_type::WHITE_PAWN);
			boardState.SetPiece<true>(to, piece_type::NONE);
		}
		else {
			boardState.SetPiece<false>(to - forward<Black>, piece_type::BLACK_PAWN);
			boardState.SetPiece<true>(to, piece_type::NONE);
		}
	}
	else if (capturedPiece == piece_type::NONE) {
		boardState.SetPiece<true>(to, piece_type::NONE);
	}
	else {
		boardState.SetPiece<true>(to, piece_type::NONE);
		boardState.SetPiece<false>(to, capturedPiece | (~pieceColor & 0b1000));
	}

	PieceType promotionPiece = PromotionPiece();
	if (promotionPiece != piece_type::NONE) {
		boardState.SetPiece<false>(from, piece_type::PAWN | pieceColor);
	}
	else {
		boardState.SetPiece<false>(from, pieceType);
	}
	if (ChangedEnPassantTarget()) {
		boardState.enPassantTargets.pop();
	}

	if (ChangedCaslingRights()) {
		boardState.caslingStates.pop();
	}
	MakeMoveMoveCaslingRook<Black, true>();
}

inline void AddAllMoves(u8 start, BoardSet moveSet, Move move) {
	for (u8 i = 0; moveSet != 0; i++) {
		Location location = extractFirstOccupied(&moveSet);
		move.to = location;
		moves[start + i] = move;
	}
}

template <bool Black>
u8 generateKingMoves(u8 start) {
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
u8 generateKnightMoves(u8 start) {
	BoardSet friendlyPieces;
	BoardSet knightSet;
	BoardSet pinnedSet;
	if constexpr (Black) {
		knightSet = boardState.black.knight;
		friendlyPieces = boardState.black.all;
		pinnedSet = boardState.blackPinnedSets.named.all;
	}
	else {
		knightSet = boardState.white.knight;
		friendlyPieces = boardState.white.all;
		pinnedSet = boardState.whitePinnedSets.named.all;
	}
	u8 numberOfMoves = 0;
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
u8 generatePawnMovesWithPossiblePromotion(u8 start, Location from, Location to) {
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
__forceinline
u8 generatePawnMovesWithPossiblePromotion(u8 start, Location from, Location to) {
	Rank rank = rankOf(to);
	if (rank == 7 || rank == 0) {
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
u8 generatePawnMoves(u8 start, BoardSet pawnSet, BoardSet allPieces) {
	u8 numberOfMoves = 0;
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

template <bool Black>
bool isEnPassantPinned(Location pawnLocation) {
	BoardSet king;
	BoardSet enemyRookLikes;
	BoardSet row = enPassantLocations<Black>;
	if constexpr (Black) {
		king = boardState.black.king & row;
		enemyRookLikes = (boardState.white.rook | boardState.white.queen) & row;
	}
	else {
		king = boardState.white.king & row;
		enemyRookLikes = (boardState.black.rook | boardState.black.queen) & row;
	}
	if (king == 0 || enemyRookLikes == 0) return false;
	BoardSet allPieces = boardState.white.all | boardState.black.all;
	
	BoardSet pawn = 1ULL << pawnLocation;
	BoardSet beforePawn = (pawn - 1);
	BoardSet afterPawn = ~(pawn | beforePawn);
	if (((enemyRookLikes & afterPawn) != 0 && ((king & beforePawn) != 0))) {
		BoardSet range = bitRangeInside(king, onlyFirstBit(enemyRookLikes));
		allPieces &= range;
		return numberOfOccupancies(allPieces) <= 2;
	}
	else if (((enemyRookLikes & beforePawn) != 0 && ((king & afterPawn) != 0))) {
		BoardSet range = bitRangeInside(onlyLastBit(enemyRookLikes), king);
		allPieces &= range;
		return numberOfOccupancies(allPieces) <= 2;
	}
	return false;
}

template <bool Black, bool In_En_Passant_Locations, bool In_Promotion_Locations, File Direction>
u8 generatePawnAttacks(u8 start, BoardSet pawnSet, BoardSet enemyPieces) {
	NamedPinnedSets *pinnedSets;

	if constexpr (Black) {
		pinnedSets = &boardState.blackPinnedSets.named;
	}
	else {
		pinnedSets = &boardState.whitePinnedSets.named;
	}
	if constexpr (Direction == (File) 1) {
		if constexpr (Black) pawnSet &= ~pinnedSets->positiveDiagonal;
		else pawnSet &= ~pinnedSets->negativeDiagonal;
	}
	else {
		if constexpr (Black) pawnSet &= ~pinnedSets->negativeDiagonal;
		else pawnSet &= ~pinnedSets->positiveDiagonal;
	}

	u8 numberOfMoves = 0;
	while (pawnSet != 0) {
		Location location = extractFirstOccupied(&pawnSet);
		Location target = location + forward<Black> +Direction;
		if constexpr (In_En_Passant_Locations) {
			File targetFile = boardState.enPassantTargets.current();
			if (targetFile != NullLocation) {
				File attackedFile = fileOf(location) + (u8) Direction;
				if (targetFile == attackedFile) {
					if (!isEnPassantPinned<Black>(location)) {
						moves[start + numberOfMoves] = { location, target, piece_type::NONE };
						numberOfMoves++;
					}
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
	return allOccupiedSet & ~promotionMoveLocations<Black>;
}

template <bool Black, bool In_En_Passant_Locations, bool In_Promotion_Locations>
u8 generatePawnAttacksWithPossibleSpecialMoves(u8 start, BoardSet pawnSet, BoardSet enemyPieces) {
	constexpr BoardSet specialMovesLocations = specialMoveLocations<Black, In_En_Passant_Locations, In_Promotion_Locations>();

	u8 numberOfMoves = 0;
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, (File) -1>(start + numberOfMoves, pawnSet & ~AFile & specialMovesLocations, enemyPieces);
	numberOfMoves += generatePawnAttacks<Black, In_En_Passant_Locations, In_Promotion_Locations, (File) +1>(start + numberOfMoves, pawnSet & ~HFile & specialMovesLocations, enemyPieces);
	return numberOfMoves;
}

// Recode all pawn moves and attacks (more bit hacks (shifting for enemy set))
template <bool Black>
u8 generatePawnMovesAndAttacks(u8 start) {
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
	u8 numberOfMoves = 0;
	// attacks
	BoardSet attackPawnSet = pawnSet & ~pinnedSets->vertical;
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, false, true>(start + numberOfMoves, attackPawnSet, enemyPieces); // promotions
	numberOfMoves += generatePawnAttacksWithPossibleSpecialMoves<Black, true, false>(start + numberOfMoves, attackPawnSet, enemyPieces); // en passant
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
u8 AddAllMovesInDirection(u8 start, Location location, BoardSet enemyPieces, BoardSet friendlyPieces) {
	constexpr Location direction = (Location) (8*Rank_Direction + File_Direction);

	Location target = location;
	u8 numberOfMoves = 0;
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
u8 generateRookLikeMoves(u8 start) {
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
	rookSet &= ~pinnedSets->diagonal;
	u8 numberOfMoves = 0;
	while (rookSet != 0) {
		Location location = extractFirstOccupied(&rookSet);
		if (!isOccupied(pinnedSets->vertical, location)) {
			numberOfMoves += AddAllMovesInDirection<+1, +0>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
			numberOfMoves += AddAllMovesInDirection<-1, +0>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		}
		if (!isOccupied(pinnedSets->horizontal, location)) {
			numberOfMoves += AddAllMovesInDirection<+0, +1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
			numberOfMoves += AddAllMovesInDirection<+0, -1>(start + numberOfMoves, location, enemyPieces, friendlyPieces);
		}
	}
	return numberOfMoves;
}
template <bool Black>
u8 generateBishopLikeMoves(u8 start) {
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
	bishopSet &= ~pinnedSets->lateral;
	u8 numberOfMoves = 0;
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
template <bool Black>
u8 generateCaslingMoves(u8 start) {
	if constexpr (Black) {
		u8 numberOfMoves = 0;
		constexpr BoardSet casleKingsideMask = 0b01100000ULL << 56;
		constexpr BoardSet casleQueensideMask = 0b00001100ULL << 56;
		constexpr BoardSet casleQueensidePiecesMask = 0b00001110ULL << 56;
		BoardSet attacked = boardState.whiteAttacks.all;
		BoardSet pieces = boardState.black.all | boardState.white.all;
		if (canBlackCasleKingside(boardState.caslingStates.currentState())) {
			if (((attacked | pieces) & casleKingsideMask) == 0) {
				moves[start + numberOfMoves] = { 074, 076, 0 };
				numberOfMoves++;
			}
		}
		if (canBlackCasleQueenside(boardState.caslingStates.currentState())) {
			if (((pieces & casleQueensidePiecesMask) | (attacked & casleQueensideMask)) == 0) {
				moves[start + numberOfMoves] = { 074, 072, 0 };
				numberOfMoves++;
			}
		}
		return numberOfMoves;
	}
	else {
		u8 numberOfMoves = 0;
		constexpr BoardSet casleKingsideMask = 0b01100000ULL;
		constexpr BoardSet casleQueensideMask = 0b00001100ULL;
		constexpr BoardSet casleQueensidePiecesMask = 0b00001110ULL;
		BoardSet attacked = boardState.blackAttacks.all;
		BoardSet pieces = boardState.white.all | boardState.black.all;
		if (canWhiteCasleKingside(boardState.caslingStates.currentState())) {
			if (((attacked | pieces) & casleKingsideMask) == 0) {
				moves[start + numberOfMoves] = { 004, 006, 0 };
				numberOfMoves++;
			}
		}
		if (canWhiteCasleQueenside(boardState.caslingStates.currentState())) {
			if (((pieces & casleQueensidePiecesMask) | (attacked & casleQueensideMask)) == 0) {
				moves[start + numberOfMoves] = { 004, 002, 0 };
				numberOfMoves++;
			}
		}
		return numberOfMoves;
	}
}

inline BoardSet RayBlocks(BoardSet blockingBishopLikePieces, BoardSet blockingRookLikePieces, Location blockingLocation, BoardSet blockingLocationSet) {
	BoardSet allPieces = boardState.black.all | boardState.white.all;

	const BoardSet *rays = pinRays[blockingLocation];

	BoardSet lowBits = blockingLocationSet - 1;
	BoardSet highBits = allOccupiedSet & ~(lowBits | blockingLocationSet);

	BoardSet blockers = 0;

	boardState.checkData.checkSource = 0;
	ScanPinRay<false>(blockingBishopLikePieces, allPieces, 0, rays[0] & lowBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<true>(blockingBishopLikePieces, allPieces, 0, rays[0] & highBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<false>(blockingBishopLikePieces, allPieces, 0, rays[2] & lowBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<true>(blockingBishopLikePieces, allPieces, 0, rays[2] & highBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<false>(blockingRookLikePieces, allPieces, 0, rays[1] & lowBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<true>(blockingRookLikePieces, allPieces, 0, rays[1] & highBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<false>(blockingRookLikePieces, allPieces, 0, rays[3] & lowBits);
	blockers |= boardState.checkData.checkSource;
	ScanPinRay<true>(blockingRookLikePieces, allPieces, 0, rays[3] & highBits);
	blockers |= boardState.checkData.checkSource;
	return blockers;
}

// also capturing the checking piece
template <bool Black>
u8 GenerateBlockingMoves(u8 start) {
	CheckData &checkData = boardState.checkData;
	PieceSets pieceSets;
	BoardSet pinnedSet;
	if constexpr (Black) {
		pieceSets = boardState.black;
		pinnedSet = boardState.blackPinnedSets.named.all;
	}
	else {
		pieceSets = boardState.white;
		pinnedSet = boardState.whitePinnedSets.named.all;
	}
	BoardSet blockLocations = checkData.checkRay;
	BoardSet checkSource = checkData.checkSource;
	blockLocations = blockLocations | checkSource;
	if (blockLocations == 0) return 0;

	pieceSets.pawn &= ~pinnedSet;
	pieceSets.knight &= ~pinnedSet;
	pieceSets.bishop &= ~pinnedSet;
	pieceSets.rook &= ~pinnedSet;
	pieceSets.queen &= ~pinnedSet;

	u8 numberOfMoves = 0;

	/* pawn captures */ {
		Location checkLocation = firstOccupied(checkSource);

		File enPassantFile = boardState.enPassantTargets.current();
		if (enPassantFile != NullLocation) {
			BoardSet enemyPawns = Black? boardState.white.pawn : boardState.black.pawn;
			bool isPawnCheck = (enemyPawns & checkSource) != 0;
			if (isPawnCheck) {
				BoardSet enPassantFileSet = AFile << enPassantFile;
				BoardSet enPassantingFiles = ((enPassantFileSet & ~HFile) << 1) | ((enPassantFileSet & ~AFile) >> 1);
				BoardSet pawnCaptureLocations = pieceSets.pawn & enPassantLocations<Black> & enPassantingFiles;
				while (pawnCaptureLocations != 0) {
					Location location = extractFirstOccupied(&pawnCaptureLocations);
					moves[start + numberOfMoves] = { location, (Location) (checkLocation + forward<Black>), piece_type::NONE };
					numberOfMoves++;
				}
			}
		}
		BoardSet pawnCaptureLocations;

		if constexpr (Black) {
			pawnCaptureLocations = (((checkSource & ~AFile) << 7) | ((checkSource & ~HFile) << 9)) & pieceSets.pawn;
		}
		else {
			pawnCaptureLocations = (((checkSource & ~HFile) >> 7) | ((checkSource & ~AFile) >> 9)) & pieceSets.pawn;
		}
		while (pawnCaptureLocations != 0) {
			numberOfMoves += generatePawnMovesWithPossiblePromotion(start + numberOfMoves, extractFirstOccupied(&pawnCaptureLocations), checkLocation);
		}
	}

	BoardSet currentBlockSet;
	Location currentBlockLocation;
	while (blockLocations != 0) {
		currentBlockLocation = firstOccupied(blockLocations);
		currentBlockSet = 1ULL << currentBlockLocation;
		blockLocations &= ~currentBlockSet;

		/* pawn blocks */ {
			BoardSet allPieces = boardState.black.all | boardState.white.all;
			BoardSet onlyBlockLocations = currentBlockSet & ~checkSource;
			BoardSet pawnBlockLocations;
			if constexpr (Black) {
				pawnBlockLocations = ((onlyBlockLocations << 8) | ((onlyBlockLocations << 16) & twoMoveLocations<Black> & ~(allPieces << 8))) & pieceSets.pawn;
			}
			else {
				pawnBlockLocations = ((onlyBlockLocations >> 8) | ((onlyBlockLocations >> 16) & twoMoveLocations<Black> & ~(allPieces >> 8))) & pieceSets.pawn;
			}
			while (pawnBlockLocations != 0) {
				numberOfMoves += generatePawnMovesWithPossiblePromotion(start + numberOfMoves, extractFirstOccupied(&pawnBlockLocations), currentBlockLocation);
			}
		}
		/* knight blocks */ {
			BoardSet knightBlocks = knightMoves[currentBlockLocation] & pieceSets.knight;
			while (knightBlocks != 0) {
				Location knightLocation = extractFirstOccupied(&knightBlocks);
				moves[start + numberOfMoves] = { knightLocation, currentBlockLocation, 0 };
				numberOfMoves++;
			}
		}
		/* bishop like and rook like blocks */ {
			BoardSet blockingPieces = RayBlocks(pieceSets.bishop | pieceSets.queen, pieceSets.rook | pieceSets.queen, currentBlockLocation, currentBlockSet);
			blockingPieces &= ~checkSource;
			while (blockingPieces != 0) {
				Location location = extractFirstOccupied(&blockingPieces);
				moves[start + numberOfMoves] = { location, currentBlockLocation, 0 };
				numberOfMoves++;
			}
		}
	}
	return numberOfMoves;
}

template <bool Black>
u8 GenerateMoves() {
	if (moves.start + 350 > moves.capacity) {
		moves.resizeAdd();
	}
	GeneratePinnedSets<Black>();
	CheckUncheckedChecks<Black>();

	u8 movesSize = 0;
	if (boardState.checkData.checkCount == 0) {
		movesSize += generateCaslingMoves<Black>(movesSize);
		movesSize += generatePawnMovesAndAttacks<Black>(movesSize);
		movesSize += generateKnightMoves<Black>(movesSize);
		movesSize += generateBishopLikeMoves<Black>(movesSize);
		movesSize += generateRookLikeMoves<Black>(movesSize);
	}
	else if (boardState.checkData.checkCount == 1) {
		movesSize += GenerateBlockingMoves<Black>(movesSize);
	}
	movesSize += generateKingMoves<Black>(movesSize);
	return movesSize;
}

