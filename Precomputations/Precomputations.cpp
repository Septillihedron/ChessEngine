
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
    std::cout << "{\n";
    for (int pieceType = 0; pieceType<12; pieceType++) {
        std::cout << "  {";
        for (int loc = 0; loc<64; loc++) {
            if (loc & 7 == 0) std::cout << "\n    ";
            Hash hash = random(gen);
            std::cout << HashToString(hash) << ", ";
        }
        std::cout << "  }, \n";
    }
    std::cout << "};\n";
}

int main()
{
    Hashes();
}

