#pragma once

#include <cstdlib>
#include <stdint.h>
#include <vector>
#include "Parameters.h"
#include "BoardRepresentation.h"
#include "MoveSets.h"

// 00TTTPPP
// T = move type
// P = promotion piece
typedef u8 MoveMetadata;

typedef int16_t i16;

enum MoveTypes : MoveMetadata {
	NORMAL = 0 << 6,
	EN_PASSANT = 1 << 6,
	CASLING = 2 << 6,
	PROMOTION = 3 << 6
};

typedef struct Move {
	union {
		struct {
			Location fromAndType;
			Location toAndPromotion;
		};
		u16 move;
	};

	constexpr Move() : move(0) {}

	constexpr Move(u16 x) : move(x) {}
	
	template<MoveTypes type = MoveTypes::NORMAL, PieceType promotionType = piece_type::KNIGHT>
	__forceinline
	static Move Make(Location from, Location to) {
		constexpr PieceType promotionMask = (promotionType - piece_type::KNIGHT) << 6;
		return Move((from | type) | ((u16) (to | promotionMask) << 8));
	}

	__forceinline int From() {
		return fromAndType & 0b00111111;
	}
	__forceinline int To() {
		return toAndPromotion & 0b00111111;
	}

	__forceinline PieceType PromotionPiece() {
		return (toAndPromotion >> 6) + piece_type::KNIGHT;
	}
	__forceinline MoveTypes MoveType() {
		return (MoveTypes) (fromAndType & 0b11000000);
	}

	void MakeMoveChangeCaslingRights(BoardSet toAndFromMask, int from, int to);
	template <bool Black, bool Reversed>
	void MakeMoveMoveCaslingRook(int to);

	template <bool Black>
	void Make();
	inline void Make(bool Black) {
		if (Black) Make<true>();
		else Make<false>();
	}
	template <bool Black>
	void Unmake(State prevState);

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
		int from = From();
		int to = To();
		u8 fromFile = fileOf(from);
		u8 fromRank = rankOf(from);
		u8 toFile = fileOf(to);
		u8 toRank = rankOf(to);

		std::string s;
		s += (char) (fromFile + 'a');
		s += (char) (fromRank + '1');
		s += (char) (toFile + 'a');
		s += (char) (toRank + '1');
		if (MoveType() == PROMOTION) {
			s += PromotionPiece(PromotionPiece());
		}
		return s;
	}

	inline bool operator==(Move &other) {
		return move == other.move;
	}
} Move;

typedef struct MovesArray {
	u16 capacity;
	u16 start;
	u16 sizes[_Max_Search_Depth_];
	u8 sizesIndex;
	Move *moves;

	MovesArray();

	__forceinline void push(u16 size) {
		sizes[sizesIndex] = size;
		sizesIndex++;
		start += size;
	}
	__forceinline void pop() {
		sizesIndex--;
		start -= sizes[sizesIndex];
	}
	__forceinline void resizeAdd() {
		capacity += 350;
		//std::cout << "Moves array size resized to: " << capacity << std::endl;
		moves = (Move *) realloc(moves, capacity * sizeof(Move));
	}
	__forceinline void resizeSub() {
		capacity -= 350;
		moves = (Move *) realloc(moves, capacity * sizeof(Move));
	}

	__forceinline Move &operator[](u8 index) {
		return this->moves[this->start + index];
	}

} MovesArray;

extern MovesArray moves;
extern std::vector<Move> movesPlayed;

extern int captures;

inline void Move::MakeMoveChangeCaslingRights(BoardSet toAndFromMask, int from, int to) {
	CaslingState &caslingState = boardState.state.casling;

	if (caslingState == 0) {
		return;
	}

	if ((toAndFromMask & (0b10010001ULL | 0b10010001ULL << 56)) == 0) {
		return;
	}

	if (from == 000 || to == 000) caslingState &= 0b1101;
	if (from == 007 || to == 007) caslingState &= 0b1110;
	if (from == 070 || to == 070) caslingState &= 0b0111;
	if (from == 077 || to == 077) caslingState &= 0b1011;

	if (from == 0004) caslingState &= 0b1100;
	if (from == 0074) caslingState &= 0b0011;
}

template <bool Black, bool Reversed>
void Move::MakeMoveMoveCaslingRook(int to) {
	if constexpr (Black) {
		constexpr Location kingRookFrom = 077;
		constexpr Location kingRookTo = 075;
		constexpr Location queenRookFrom = 070;
		constexpr Location queenRookTo = 073;
		constexpr BoardSet kingXorMask = (1ULL << kingRookFrom) | (1ULL << kingRookTo);
		constexpr BoardSet queenXorMask = (1ULL << queenRookFrom) | (1ULL << queenRookTo);

		Location clear;
		Location place;
		BoardSet xorMask;
		if (to == 076) {
			clear = (!Reversed)? kingRookFrom : kingRookTo;
			place = (!Reversed)? kingRookTo : kingRookFrom;
			xorMask = kingXorMask;
		}
		else {
			clear = (!Reversed)? queenRookFrom : queenRookTo;
			place = (!Reversed)? queenRookTo : queenRookFrom;
			xorMask = queenXorMask;
		}

		boardState.black.all ^= xorMask;
		boardState.black.rook ^= xorMask;
		boardState.squares[clear] = piece_type::NONE;
		boardState.squares[place] = piece_type::BLACK_ROOK;
	}
	else {
		constexpr Location kingRookFrom = 007;
		constexpr Location kingRookTo = 005;
		constexpr Location queenRookFrom = 000;
		constexpr Location queenRookTo = 003;
		constexpr BoardSet kingXorMask = (1ULL << kingRookFrom) | (1ULL << kingRookTo);
		constexpr BoardSet queenXorMask = (1ULL << queenRookFrom) | (1ULL << queenRookTo);

		Location clear;
		Location place;
		BoardSet xorMask;
		if (to == 006) {
			clear = (!Reversed)? kingRookFrom : kingRookTo;
			place = (!Reversed)? kingRookTo : kingRookFrom;
			xorMask = kingXorMask;
		}
		else {
			clear = (!Reversed)? queenRookFrom : queenRookTo;
			place = (!Reversed)? queenRookTo : queenRookFrom;
			xorMask = queenXorMask;
		}

		boardState.white.all ^= xorMask;
		boardState.white.rook ^= xorMask;
		boardState.squares[clear] = piece_type::NONE;
		boardState.squares[place] = piece_type::WHITE_ROOK;
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
// includes allPieces/blockingPieces
__forceinline
BoardSet __RayMoveSet(BoardSet highBits, BoardSet lowBits, BoardSet allPieces) {
	BoardSet highRayPieces = highBits & allPieces;
	BoardSet lowRayLastPiece = onlyLastBit(lowBits & allPieces);
	BoardSet highRay = belowLS1B<true>(highRayPieces) & highBits;
	BoardSet lowRay = (lowRayLastPiece == 0)? lowBits : ~(lowRayLastPiece - 1) & lowBits;

	return highRay | lowRay;
}
template <bool Laterals, bool Diagonals>
__inline
BoardSet RaysMoveSet(BoardSet pieceSet, BoardSet blockingPieces) {
	BoardSet moves = 0;
	while (pieceSet != 0) {
		BoardSet lowBits = belowLS1B(pieceSet);
		BoardSet highBits = aboveLS1B(pieceSet);
		Location location = extractFirstOccupied(&pieceSet);
		const BoardSet *rays = pinRays[location];

		if constexpr (Laterals) {
			moves |= __RayMoveSet(highBits & rays[1], lowBits & rays[1], blockingPieces);
			moves |= __RayMoveSet(highBits & rays[3], lowBits & rays[3], blockingPieces);
		}
		if constexpr (Diagonals) {
			moves |= __RayMoveSet(highBits & rays[0], lowBits & rays[0], blockingPieces);
			moves |= __RayMoveSet(highBits & rays[2], lowBits & rays[2], blockingPieces);
		}

	}
	return moves;
}
__forceinline
BoardSet kingMoveSet(BoardSet pieceSet) {
	return kingMoves[firstOccupied(pieceSet)];
}

template <bool After_King>
__inline
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
	BoardSet &enemyKing = Black? boardState.white.king : boardState.black.king;
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
	defendSets->king = attackSets->king = kingMoveSet(pieceSets->king);
	attackSets->pawn = pawnAttackSet<Black>(pieceSets->pawn);
	defendSets->knight = attackSets->knight = knightMoveSet(pieceSets->knight);
	defendSets->pawn = pawnMoveSet<Black>(pieceSets->pawn);
	BoardSet blockingPieces = (boardState.black.all | boardState.white.all) & ~enemyKing;
	defendSets->bishop = attackSets->bishop = RaysMoveSet<false, true>(pieceSets->bishop, blockingPieces);
	defendSets->rook = attackSets->rook = RaysMoveSet<true, false>(pieceSets->rook, blockingPieces);
	defendSets->queen = attackSets->queen = RaysMoveSet<true, true>(pieceSets->queen, blockingPieces);

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

	pinnedSets.negativeDiagonal = ScanPinRay<false>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[0] & beforeKing);
	pinnedSets.negativeDiagonal |= ScanPinRay<true>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[0] & afterKing);
	pinnedSets.positiveDiagonal = ScanPinRay<false>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[2] & beforeKing);
	pinnedSets.positiveDiagonal |= ScanPinRay<true>(enemyBishopLike, enemyNonBishopLike, friendlyPieces, pinRay[2] & afterKing);

	pinnedSets.vertical = ScanPinRay<false>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[1] & beforeKing);
	pinnedSets.vertical |= ScanPinRay<true>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[1] & afterKing);
	pinnedSets.horizontal = ScanPinRay<false>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[3] & beforeKing);
	pinnedSets.horizontal |= ScanPinRay<true>(enemyRookLike, enemyNonRookLike, friendlyPieces, pinRay[3] & afterKing);

	pinnedSets.diagonal = pinnedSets.negativeDiagonal | pinnedSets.positiveDiagonal;
	pinnedSets.lateral = pinnedSets.vertical | pinnedSets.horizontal;

	pinnedSets.all = pinnedSets.diagonal | pinnedSets.lateral;
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
	movesPlayed.push_back(*this);

	int from = From();
	int to = To();

	PieceType movingPiece = boardState.squares[from];
	PieceType captured = boardState.squares[to];

	PieceSets &friendPieceSets = Black? boardState.black : boardState.white;
	PieceSets &enemyPieceSets = Black? boardState.white : boardState.black;

	BoardSet &movingPieceSet = boardState.pieces[movingPiece];

	BoardSet toMask = 1ULL << to;
	BoardSet fromMask = 1ULL << from;
	BoardSet toAndFromMask = toMask | fromMask;

	MoveTypes moveType = MoveType();

	boardState.state.enPassantFile = NullLocation;
	boardState.state.captured = piece_type::NONE;

	switch (moveType) {
	case NORMAL:
		// capture
		boardState.pieces[captured] &= ~toMask;
		enemyPieceSets.all &= ~toMask;
		boardState.state.captured = captured;
		// move
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= toAndFromMask;
		boardState.squares[from] = piece_type::NONE;
		boardState.squares[to] = movingPiece;

		if (movingPiece == piece_type::COLORED_PAWN<Black>) {
			if (to - from == (Black? -16 : 16)) {
				boardState.state.enPassantFile = fileOf(to);
			}
		}
		
		MakeMoveChangeCaslingRights(toAndFromMask, from, to);
		break;
	case EN_PASSANT:
		// capture
		enemyPieceSets.pawn &= ~(Black? toMask << 8 : toMask >> 8);
		enemyPieceSets.all &= ~(Black? toMask << 8 : toMask >> 8);
		boardState.squares[Black? to + 8 : to - 8] = piece_type::NONE;
		// move
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= toAndFromMask;
		boardState.squares[from] = piece_type::NONE;
		boardState.squares[to] = movingPiece;
		break;
	case CASLING:
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= toAndFromMask;
		boardState.squares[from] = piece_type::NONE;
		boardState.squares[to] = movingPiece;
		MakeMoveMoveCaslingRook<Black, false>(to);

		boardState.state.casling &= Black? 0b0011 : 0b1100;
		break;
	case PROMOTION:
		// capture
		boardState.pieces[captured] &= ~toMask;
		enemyPieceSets.all &= ~toMask;
		boardState.state.captured = captured;
		// remove pawn
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= fromMask;
		boardState.squares[from] = piece_type::NONE;
		// promotion
		PieceType promotionPiece = PromotionPiece();
		promotionPiece |= piece_type::COLOR<Black>;
		boardState.pieces[promotionPiece] ^= toMask;
		boardState.squares[to] = promotionPiece;

		MakeMoveChangeCaslingRights(toAndFromMask, from, to);
		break;
	}

	if constexpr (_Strict_) {
		if (boardState.black.king == 0 || boardState.white.king == 0) throw std::invalid_argument("king captured");
	}
}
template <bool Black>
void Move::Unmake(State prevState) {
	movesPlayed.pop_back();

	int from = From();
	int to = To();

	PieceType movingPiece = boardState.squares[to];
	PieceType movingPiece_u = uncoloredType(movingPiece);

	PieceSets &friendPieceSets = Black? boardState.black : boardState.white;
	PieceSets &enemyPieceSets = Black? boardState.white : boardState.black;

	BoardSet &movingPieceSet = friendPieceSets.pieces[movingPiece_u];

	BoardSet toMask = 1ULL << to;
	BoardSet fromMask = 1ULL << from;
	BoardSet toAndFromMask = toMask | fromMask;

	MoveTypes moveType = MoveType();
	PieceType captured = boardState.state.captured;

	boardState.state = prevState;

	switch (moveType) {
	case NORMAL:
		// capture
		boardState.pieces[captured] |= toMask;
		enemyPieceSets.all |= toMask * (captured > 0);
		boardState.squares[to] = captured;
		// move
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= toAndFromMask;
		boardState.squares[from] = movingPiece;
		break;
	case EN_PASSANT:
		// capture
		enemyPieceSets.pawn |= Black? toMask << 8 : toMask >> 8;
		enemyPieceSets.all |= Black? toMask << 8 : toMask >> 8;
		boardState.squares[Black? to + 8 : to - 8] = piece_type::COLORED_PAWN<!Black>;
		// move
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= toAndFromMask;
		boardState.squares[to] = piece_type::NONE;
		boardState.squares[from] = movingPiece;
		break;
	case CASLING:
		friendPieceSets.all ^= toAndFromMask;
		movingPieceSet ^= toAndFromMask;
		boardState.squares[to] = piece_type::NONE;
		boardState.squares[from] = movingPiece;
		MakeMoveMoveCaslingRook<Black, true>(to);
		break;
	case PROMOTION:
		// capture
		boardState.pieces[captured] |= toMask;
		enemyPieceSets.all |= toMask * (captured > 0);
		boardState.squares[to] = captured;
		// remove pawn
		friendPieceSets.all ^= toAndFromMask;
		friendPieceSets.pawn ^= fromMask;
		boardState.squares[from] = piece_type::COLORED_PAWN<Black>;
		// promotion
		PieceType promotionPiece = PromotionPiece();
		promotionPiece |= piece_type::COLOR<Black>;
		movingPieceSet ^= toMask;
		break;
	}
}

__forceinline
void AddAllMoves(u8 start, BoardSet moveSet, Move move) {
	for (u8 numberOfMoves = 0; moveSet != 0; numberOfMoves++) {
		Location location = extractFirstOccupied(&moveSet);
		move.toAndPromotion = location;
		moves[start + numberOfMoves] = move;
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
	AddAllMoves(start, kingMoveLocations, Move::Make(location, 0));
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
		pinnedSet = boardState.blackPinnedSets.all;
	}
	else {
		knightSet = boardState.white.knight;
		friendlyPieces = boardState.white.all;
		pinnedSet = boardState.whitePinnedSets.all;
	}
	u8 numberOfMoves = 0;
	knightSet &= ~pinnedSet;
	while (knightSet != 0) {
		Location location = extractFirstOccupied(&knightSet);
		BoardSet knightMoveLocations = knightMoves[location] & ~friendlyPieces;
		AddAllMoves(start + numberOfMoves, knightMoveLocations, Move::Make(location, 0));
		numberOfMoves += numberOfOccupancies(knightMoveLocations);
	}
	return numberOfMoves;
}

__forceinline
void generatePawnPromotionMoves(u8 start, Location from, Location to) {
	moves[start + 0] = Move::Make<PROMOTION, piece_type::QUEEN>(from, to);
	moves[start + 1] = Move::Make<PROMOTION, piece_type::ROOK>(from, to);
	moves[start + 2] = Move::Make<PROMOTION, piece_type::BISHOP>(from, to);
	moves[start + 3] = Move::Make<PROMOTION, piece_type::KNIGHT>(from, to);
}

template <bool In_Promotion_Locations>
__forceinline
u8 generatePawnMovesWithPossiblePromotion(u8 start, Location from, Location to) {
	if constexpr (In_Promotion_Locations) {
		generatePawnPromotionMoves(start, from, to);
		return 4;
	}
	else {
		moves[start] = Move::Make(from, to);
		return 1;
	}
}
__forceinline
u8 generatePawnMovesWithPossiblePromotion(u8 start, Location from, Location to) {
	Rank rank = rankOf(to);
	if (rank == 7 || rank == 0) {
		generatePawnPromotionMoves(start, from, to);
		return 4;
	}
	else {
		moves[start] = Move::Make(from, to);
		return 1;
	}
}

template <bool Black>
constexpr BoardSet movePawnForward(BoardSet pawnSet) {
	if constexpr (Black) {
		return pawnSet >> 8;
	}
	else {
		return pawnSet << 8;
	}
}
template <bool Negative>
constexpr BoardSet movePawnSideways(BoardSet pawnSet) {
	if constexpr (Negative) {
		return (pawnSet >> 1) & ~HFile;
	}
	else {
		return (pawnSet << 1) & ~AFile;
	}
}

template <bool Black>
u8 generatePawnMoves(u8 start, BoardSet pawnSet, BoardSet allPieces) {
	constexpr BoardSet twoMoveTargetLocations = movePawnForward<Black>(movePawnForward<Black>(twoMoveLocations<Black>));
	constexpr BoardSet promotionTargetLocations = movePawnForward<Black>(promotionMoveLocations<Black>);
	BoardSet firstStep = movePawnForward<Black>(pawnSet) & ~allPieces;
	BoardSet promotionMovers = firstStep & promotionTargetLocations;
	BoardSet twoMovers = movePawnForward<Black>(firstStep) & ~allPieces & twoMoveTargetLocations;
	BoardSet oneMovers = firstStep & ~promotionTargetLocations;

	constexpr Location backwards = (u8) -forward<Black>;
	u8 numberOfMoves = 0;
	// promotions
	while (promotionMovers != 0) {
		Location target = extractFirstOccupied(&promotionMovers);
		Location from = (u8) (target + backwards);
		generatePawnPromotionMoves(start + numberOfMoves, from, target);
		numberOfMoves += 4;
	}
	// double movers
	while (twoMovers != 0) {
		constexpr Location doubleBackwards = (u8) (2*backwards);
		Location target = extractFirstOccupied(&twoMovers);
		Location from = (u8) (target + doubleBackwards);
		moves[start + numberOfMoves] = Move::Make(from, target);
		numberOfMoves += 1;
	}
	// one movers
	while (oneMovers != 0) {
		Location target = extractFirstOccupied(&oneMovers);
		Location from = (u8) (target + backwards);
		moves[start + numberOfMoves] = Move::Make(from, target);
		numberOfMoves += 1;
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
template <bool Black, bool Negative, bool Promotions>
__forceinline
u8 addPawnAttacks(u8 start, BoardSet attacks) {
	constexpr Location offset = (Location) (forward<Black> + ((Black ^ Negative)? -1 : 1));
	u8 numberOfMoves = 0;
	while (attacks != 0) {
		Location target = extractFirstOccupied(&attacks);
		numberOfMoves += generatePawnMovesWithPossiblePromotion<Promotions>(start + numberOfMoves, (Location) (target - offset), target);
	}
	return numberOfMoves;
}
template <bool Black>
u8 generateEnPassants(u8 start, BoardSet negativeAttacks, BoardSet positiveAttacks) {
	File enPassantFile = boardState.state.enPassantFile;
	if (enPassantFile == NullLocation) return 0;
	if constexpr (Black) {
		// make sure that negative attacks is going torwards the A file
		std::swap(negativeAttacks, positiveAttacks);
	}
	Location targetLocation = (Location) (((Black)? 2*8 : 5*8) + enPassantFile);

	u8 numberOfMoves = 0;

	if (isOccupied(negativeAttacks, targetLocation)) {
		Location from = (Location) (targetLocation - forward<Black> +1);
		if (!isEnPassantPinned<Black>(from)) {
			moves[start + numberOfMoves] = Move::Make<EN_PASSANT>(from, targetLocation);
			numberOfMoves++;
		}
	}
	if (isOccupied(positiveAttacks, targetLocation)) {
		Location from = (Location) (targetLocation - forward<Black> -1);
		if (!isEnPassantPinned<Black>(from)) {
			moves[start + numberOfMoves] = Move::Make<EN_PASSANT>(from, targetLocation);
			numberOfMoves++;
		}
	}
	return numberOfMoves;
}
template <bool Black>
u8 generatePawnAttacks(u8 start, BoardSet pawnSet, BoardSet enemyPieces, PinnedSets &pinnedSets) {
	constexpr BoardSet promotionTargetLocations = movePawnForward<Black>(promotionMoveLocations<Black>);

	u8 numberOfMoves = 0;

	BoardSet negativeAttacks = movePawnSideways<!Black>(movePawnForward<Black>(pawnSet & ~pinnedSets.positiveDiagonal));
	BoardSet positiveAttacks = movePawnSideways<Black>(movePawnForward<Black>(pawnSet & ~pinnedSets.negativeDiagonal));

	numberOfMoves += generateEnPassants<Black>(start + numberOfMoves, negativeAttacks, positiveAttacks);

	negativeAttacks &= enemyPieces;
	positiveAttacks &= enemyPieces;

	BoardSet negativeAttackPromotions = negativeAttacks & promotionTargetLocations;
	BoardSet positiveAttackPromotions = positiveAttacks & promotionTargetLocations;

	negativeAttacks &= ~promotionTargetLocations;
	positiveAttacks &= ~promotionTargetLocations;

	// promotions attacks
	numberOfMoves += addPawnAttacks<Black, true, true>(start + numberOfMoves, negativeAttackPromotions);
	numberOfMoves += addPawnAttacks<Black, false, true>(start + numberOfMoves, positiveAttackPromotions);
	// normal attacks
	numberOfMoves += addPawnAttacks<Black, true, false>(start + numberOfMoves, negativeAttacks);
	numberOfMoves += addPawnAttacks<Black, false, false>(start + numberOfMoves, positiveAttacks);

	return numberOfMoves;
}

template <bool Black>
u8 generatePawnMovesAndAttacks(u8 start) {
	BoardSet friendlyPieces;
	BoardSet enemyPieces;
	BoardSet allPieces;
	BoardSet pawnSet;
	PinnedSets &pinnedSets = (Black)? boardState.blackPinnedSets : boardState.whitePinnedSets;
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
	pawnSet &= ~pinnedSets.horizontal;
	u8 numberOfMoves = 0;

	BoardSet attackPawnSet = pawnSet & ~pinnedSets.vertical;
	numberOfMoves += generatePawnAttacks<Black>(start + numberOfMoves, attackPawnSet, enemyPieces, pinnedSets);
	BoardSet movingPawnSet = pawnSet & ~(pinnedSets.negativeDiagonal | pinnedSets.positiveDiagonal);
	numberOfMoves += generatePawnMoves<Black>(start + numberOfMoves, movingPawnSet, allPieces);

	return numberOfMoves;
}

template <int File_Direction, int Rank_Direction>
bool IsOnEdge(Location location) {
	if constexpr (File_Direction == -1) {
		if (fileOf(location) == 0) return true;
	}
	else if constexpr (File_Direction == 1) {
		if (fileOf(location) == 7) return true;
	}
	if constexpr (Rank_Direction == -1) {
		if (rankOf(location) == 0) return true;
	}
	else if constexpr (Rank_Direction == 1) {
		if (rankOf(location) == 7) return true;
	}
	return false;
}

template <int File_Direction, int Rank_Direction>
u8 AddAllMovesInDirection(u8 start, Location location, BoardSet enemyPieces, BoardSet friendlyPieces) {
	constexpr int direction = 8*Rank_Direction + File_Direction;

	Location target = location;
	u8 numberOfMoves = 0;
	while (!IsOnEdge<File_Direction, Rank_Direction>(target)) {
		target += direction;

		if (isOccupied(friendlyPieces, target)) break;

		moves[start + numberOfMoves] = Move::Make(location, target);
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
	PinnedSets *pinnedSets;
	if constexpr (Black) {
		rookSet = boardState.black.rook | boardState.black.queen;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
		pinnedSets = &boardState.blackPinnedSets;
	}
	else {
		rookSet = boardState.white.rook | boardState.white.queen;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
		pinnedSets = &boardState.whitePinnedSets;
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
	PinnedSets *pinnedSets;
	if constexpr (Black) {
		bishopSet = boardState.black.bishop | boardState.black.queen;
		friendlyPieces = boardState.black.all;
		enemyPieces = boardState.white.all;
		pinnedSets = &boardState.blackPinnedSets;
	}
	else {
		bishopSet = boardState.white.bishop | boardState.white.queen;
		friendlyPieces = boardState.white.all;
		enemyPieces = boardState.black.all;
		pinnedSets = &boardState.whitePinnedSets;
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
	CaslingState state = boardState.state.casling;
	if constexpr (Black) {
		u8 numberOfMoves = 0;
		constexpr BoardSet casleKingsideMask = 0b01100000ULL << 56;
		constexpr BoardSet casleQueensideMask = 0b00001100ULL << 56;
		constexpr BoardSet casleQueensidePiecesMask = 0b00001110ULL << 56;
		BoardSet attacked = boardState.whiteAttacks.all;
		BoardSet pieces = boardState.black.all | boardState.white.all;
		if (canBlackCasleKingside(state)) {
			if (((attacked | pieces) & casleKingsideMask) == 0) {
				moves[start + numberOfMoves] = Move::Make<CASLING>(074, 076);
				numberOfMoves++;
			}
		}
		if (canBlackCasleQueenside(state)) {
			if (((pieces & casleQueensidePiecesMask) | (attacked & casleQueensideMask)) == 0) {
				moves[start + numberOfMoves] = Move::Make<CASLING>(074, 072);
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
		if (canWhiteCasleKingside(state)) {
			if (((attacked | pieces) & casleKingsideMask) == 0) {
				moves[start + numberOfMoves] = Move::Make<CASLING>(004, 006);
				numberOfMoves++;
			}
		}
		if (canWhiteCasleQueenside(state)) {
			if (((pieces & casleQueensidePiecesMask) | (attacked & casleQueensideMask)) == 0) {
				moves[start + numberOfMoves] = Move::Make<CASLING>(004, 002);
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
		pinnedSet = boardState.blackPinnedSets.all;
	}
	else {
		pieceSets = boardState.white;
		pinnedSet = boardState.whitePinnedSets.all;
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

		File enPassantFile = boardState.state.enPassantFile;
		if (enPassantFile != NullLocation) {
			BoardSet enemyPawns = Black? boardState.white.pawn : boardState.black.pawn;
			bool isPawnCheck = (enemyPawns & checkSource) != 0;
			if (isPawnCheck) {
				BoardSet enPassantFileSet = AFile << enPassantFile;
				BoardSet enPassantingFiles = ((enPassantFileSet & ~HFile) << 1) | ((enPassantFileSet & ~AFile) >> 1);
				BoardSet pawnCaptureLocations = pieceSets.pawn & enPassantLocations<Black> & enPassantingFiles;
				while (pawnCaptureLocations != 0) {
					Location location = extractFirstOccupied(&pawnCaptureLocations);
					moves[start + numberOfMoves] = Move::Make<EN_PASSANT>(location, (Location) (checkLocation + forward<Black>));
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
				moves[start + numberOfMoves] = Move::Make(knightLocation, currentBlockLocation);
				numberOfMoves++;
			}
		}
		/* bishop like and rook like blocks */ {
			BoardSet blockingPieces = RayBlocks(pieceSets.bishop | pieceSets.queen, pieceSets.rook | pieceSets.queen, currentBlockLocation, currentBlockSet);
			blockingPieces &= ~checkSource;
			while (blockingPieces != 0) {
				Location location = extractFirstOccupied(&blockingPieces);
				moves[start + numberOfMoves] = Move::Make(location, currentBlockLocation);
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
	UpdateAttackAndDefendSets<!Black>();
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

