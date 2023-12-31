#pragma once

#include "BoardRepresentation.h"

__forceinline
bool bitAt(BoardSet set, Location loc) {
	return (set >> loc) & 1;
}
__forceinline
bool isOccupied(BoardSet set, Location loc) {
	return (set >> loc) & 1;
}
template <bool Bit>
__forceinline
void setBit(BoardSet *set, Location loc) {
	if constexpr (Bit) {
		*set |= 1ULL << loc;
	}
	else {
		*set &= ~(1ULL << loc);
	}
}
__forceinline
void flipBit(BoardSet *set, Location loc) {
	*set ^= 1ULL << loc;
}
__forceinline
BoardSet bitRange(BoardSet start, BoardSet end) {
	return ((end - 1) & ~(start - 1)) | end;
}
__forceinline
BoardSet bitRange(Location start, Location end) {
	return bitRange(1ULL << start, 1ULL << end);
}
__forceinline
BoardSet bitRangeInside(BoardSet start, BoardSet end) {
	return ((end - 1) & ~(start - 1)) & ~start;
}
__forceinline
BoardSet bitRangeInside(Location start, Location end) {
	return bitRangeInside(1ULL << start, 1ULL << end);
}
__forceinline
Location firstOccupied(BoardSet set) {
	if constexpr (_Strict_) {
		if (set == 0) throw std::invalid_argument("set is empty (equal to 0)");
	}
	unsigned long index = 0;
	_BitScanForward64(&index, set);
	Location location = (Location) index;
	return location;
}
__forceinline
Location lastOccupied(BoardSet set) {
	if constexpr (_Strict_) {
		if (set == 0) throw std::invalid_argument("set is empty (equal to 0)");
	}
	unsigned long index = 0;
	_BitScanReverse64(&index, set);
	Location location = (Location) index;
	return location;
}
__forceinline
Location extractFirstOccupied(BoardSet *set) {
	if constexpr (_Strict_) {
		if (set == 0) throw std::invalid_argument("set is empty (equal to 0)");
	}
	unsigned long index = 0;
	_BitScanForward64(&index, *set);
	*set &= (*set - 1);
	Location location = (Location) index;
	return location;
}
__forceinline
BoardSet onlyFirstBit(BoardSet set) {
	return set & (0-set);
}
__forceinline
BoardSet onlyLastBit(BoardSet set) {
	if (set == 0) return 0;
	return 1ULL << lastOccupied(set);
}
__forceinline
BoardSet aboveLS1B(BoardSet x) {
	return x ^ (0-x);
}
template <bool Include_LS1B = false>
__forceinline
BoardSet belowLS1B(BoardSet x) {
	if constexpr (Include_LS1B) {
		return x ^ (x - 1);
	}
	else {
		return ~x & (x - 1);
	}
}
__forceinline
u8 numberOfOccupancies(BoardSet set) {
	return (u8) __popcnt64(set);
}