#include "MoveGen.h"

MovesArray::MovesArray() {
	sizesIndex = 0;
	start = 0;
	capacity = 350;
	moves = (Move *) malloc(capacity * sizeof(Move));
}

MovesArray moves = MovesArray();
std::vector<Move> movesPlayed;
int captures;

