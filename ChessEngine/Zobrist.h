#pragma once

#include <string>
#include <vector>
#include <xmmintrin.h>
#include "Types.h"
#include "Parameters.h"

inline int collitions0 = 0;
inline int collitions1 = 0;

typedef struct HashedPositions {
	HashEntry pos[_Hashed_Positions_Size_];

	inline HashedPositions() {
		memset(pos, 0, sizeof(pos));
	}

	__forceinline void clear() {
		memset(pos, 0, sizeof(pos));
	}

	__forceinline void set(Hash hash, PositionValue posVal, u16 depth) {
		HashEntry &entry = pos[hash & (_Hashed_Positions_Size_ - 1)];

		if (depth < entry.depth) return;

		if (hash != entry.hash) collitions0++;
		else if (posVal != entry.value && depth != entry.depth) collitions1++;

		entry = { hash, posVal, depth };
	}
	__forceinline int index(Hash hash) {
		return hash & (_Hashed_Positions_Size_ - 1);
	}
	__forceinline void prefetch(Hash hash) {
		_mm_prefetch((const char *) &pos[index(hash)], _MM_HINT_T0);
	}
	__forceinline HashEntry get(Hash hash) {
		return pos[index(hash)];
	}

} HashedPositions;

extern Hash colorHash;
extern Hash caslingHashes[16];
extern Hash pieceHashes[12][64];

inline HashedPositions hashedPositions;

