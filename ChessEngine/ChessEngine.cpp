// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "DebugTools.h"

int main()
{
    std::cout << "Hello World!\n";
#ifdef DEBUG
    bool isBlacksTurn = false;
    Move moves[] = {
        StringToMove("b1a3"),
    };
    for (Move move : moves) {
        move.Make(isBlacksTurn);
        max_depth--;
        isBlacksTurn = !isBlacksTurn;
    }
    //while (exploreMove(isBlacksTurn)) {}
    //if (true) return 0;
    if (isBlacksTurn) {
        std::cout << explorePerft<true>(max_depth) << std::endl;
    }
    else {
        std::cout << explorePerft<false>(max_depth) << std::endl;
    }
#else
    long long timeTotal = 0;
    for (int i = 0; i<10; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << perft<false>(depth, depth) << " nodes" << std::endl;
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        timeTotal += ms_nano.count();

        std::cout << i << ": " << ms_nano.count()/1e6 << " ms" << std::endl;
    }
    std::cout << timeTotal/1e6/10 << " ms" << std::endl;
#endif // DEBUG

    return 0;
}

