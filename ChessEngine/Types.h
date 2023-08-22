#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef u64 BoardSet;
typedef u8 ChangeCaslingMask;
typedef u8 CaslingState;
typedef u8 Location;
typedef u8 PieceType;
typedef u8 File;
typedef u8 Rank;

typedef i16 PositionValue;

typedef u32 Hash;

typedef struct HashEntry {
	Hash hash;
	PositionValue value;
	u16 depth;
} HashEntry;
