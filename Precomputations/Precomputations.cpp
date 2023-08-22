
#include <iostream>
#include <format>
#include <random>
#include "Types.h"

void LowAndHighBits() {
    std::cout << "{ ";
    for (Location loc = 0; loc < 64; loc++) {
        BoardSet piece = 1ULL << loc;
        BoardSet lowBits = piece - 1;
        BoardSet highBits = ~(lowBits | piece);

        if ((loc &7) == 0) std::cout << std::endl << "    ";
        std::cout << "{ " << BoardSetToString(lowBits) << ", " << BoardSetToString(highBits) << " }, ";
    }
    std::cout << "};" << std::endl;
    std::cout << "{ ";
    for (Location loc = 0; loc < 64; loc++) {
        BoardSet piece = 1ULL << loc;
        BoardSet highBitsIP = ~(piece - 1);

        if ((loc &7) == 0) std::cout << std::endl << "    ";
        std::cout << BoardSetToString(highBitsIP) << ", ";
    }
    std::cout << "};" << std::endl;
}

void Hashes() {
    std::mt19937 gen(0);
    std::uniform_int_distribution<Hash> random(INT32_MIN, INT32_MAX);

    Hash colorHash = random(gen);

    Hash caslingHashes[16];

    for (int state = 0; state<16; state++) {
        genCaslingHash:
        Hash hash = random(gen);
        if (hash == 0) goto genCaslingHash;
        if (hash == colorHash) goto genCaslingHash;
        for (int state = 0; state<16; state++) {
            if (hash == caslingHashes[state]) goto genCaslingHash;
        }
        caslingHashes[state] = hash;
    }
    
    Hash hashes[12][64];

    for (int pieceType = 0; pieceType<12; pieceType++) {
        for (int loc = 0; loc<64; loc++) {
            genHash:
            Hash hash = random(gen);
            if (hash == 0) goto genHash;
            if (hash == colorHash) goto genHash;
            for (int state = 0; state<16; state++) {
                if (hash == caslingHashes[state]) goto genHash;
            }
            for (int pieceType = 0; pieceType<12; pieceType++) {
                for (int loc = 0; loc<64; loc++) {
                    if (hash == hashes[pieceType][loc]) goto genHash;
                }
            }
            hashes[pieceType][loc] = hash;
        }
    }

    std::cout << "#include \"Zobrist.h\"\n";

    std::cout << "\n";

    std::cout << "Hash colorHash = " << HashToString(colorHash) << ";\n\n";

    std::cout << "Hash caslingHashes[16] = {\n";
    std::cout << "    ";
    for (int state = 0; state<16; state++) {
        std::cout << HashToString(caslingHashes[state]) << ", ";
        if (state == 15) std::cout << "\n";
    }
    std::cout << "};\n";

    std::cout << "\n";

    std::cout << "Hash pieceHashes[12][64] = {\n";
    for (int pieceType = 0; pieceType<12; pieceType++) {
        std::cout << "    {";
        for (int loc = 0; loc<64; loc++) {
            if (loc % 8 == 0) std::cout << "\n        ";
            std::cout << HashToString(hashes[pieceType][loc]) << ", ";
            if (loc == 63) std::cout << "\n";
        }
        std::cout << "    }, \n";
    }
    std::cout << "};\n";
}

int main()
{
    Hashes();
}

