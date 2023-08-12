#pragma once

#include <stdint.h>
#include <format>

typedef uint64_t BoardSet;
typedef uint8_t Rank;
typedef uint8_t File;
typedef uint8_t Location;

typedef uint32_t Hash;

inline std::string BoardSetToString(BoardSet x) {
	return std::format("0x{:016x}ULL", x);
}

inline std::string HashToString(Hash x) {
	return std::format("0x{:08x}ULL", x);
}
