#include "MoveGen.h"

MovesArray::MovesArray() {
	capacity = 350;
	moves = (Move *) malloc(capacity * sizeof Move);
}

MovesArray moves = MovesArray();
int captures;

