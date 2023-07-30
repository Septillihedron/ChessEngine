
#include <iostream>
#include <format>
#include "Types.h"

int main()
{
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

