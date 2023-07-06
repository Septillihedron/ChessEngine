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

MovesArray moves = MovesArray();
int captures;

