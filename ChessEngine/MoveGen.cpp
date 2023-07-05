#include "MoveGen.h"

MovesArray::MovesArray() {
	capacity = 350;
	moves = (Move *) malloc(capacity * sizeof Move);
}
void MovesArray::push(size_t size) {
	start += size;
}
void MovesArray::pop(size_t size) {
	start -= size;
}
void MovesArray::resizeAdd() {
	capacity += 350;
	moves = (Move *) realloc(moves, capacity * sizeof Move);
}
void MovesArray::resizeSub() {
	capacity -= 350;
	moves = (Move *) realloc(moves, capacity * sizeof Move);
}

Move &MovesArray::operator[](size_t index) {
	return this->moves[this->start + index];
}

PieceType Move::PromotionPiece() {
	return metadata & 7;
}
bool Move::ChangedCaslingRights() {
	return (metadata >> 3) & 1;
}
PieceType Move::CapturedPiece() {
	return (metadata >> 4);
}

bool InRookSquares(Location location) {
	return (location == 0 || location == 7) || (location == 56 || location == 63);
}

int captures;

// make template <bool Black>
void Move::Make() {

	PieceType pieceType = boardState.squares[from];

	boardState.SetPiece<true>(from, piece_type::NONE);

	if (boardState.squares[to] != piece_type::NONE) {
		captures++;
		metadata |= uncoloredType(boardState.squares[to]) << 4;
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
	}

	PieceType promotionPiece = PromotionPiece();
	if (promotionPiece != piece_type::NONE) {
		boardState.SetPiece<false>(from, piece_type::PAWN | pieceColor);
	}
	else {
		boardState.SetPiece<false>(from, pieceType);
	}

	if (ChangedCaslingRights()) {
		boardState.caslingStates.pop();
	}
}

MovesArray moves = MovesArray();

