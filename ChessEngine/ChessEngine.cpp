// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <thread>
#include "AlphaBeta.h"
#include "DebugTools.h"

void play() {
    bool isBlack = false;
    char turn;
    std::cout << "Turn: ";
    std::cin >> turn;
    isBlack = turn == 'b';
    if (isBlack) {
        std::string responseMove;
        std::cout << "Response: ";
        std::cin >> responseMove;
        StringToMove(responseMove).Make(!isBlack);
    }
    else {
        GeneratePinnedSets<false>();
        CheckUncheckedChecks<false>();
    }
    while (true) {
        searchCanceled = false;
        bool cancelCalceler = false;
        auto cancelFunction = [&cancelCalceler]() {
            for (int times = 0; (times < 29*10) && !cancelCalceler; times++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            searchCanceled = true;
        };
        std::thread canceler(cancelFunction);

        auto t1 = std::chrono::high_resolution_clock::now();

        Move bestMove = Search(isBlack);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        std::cout << "Best move: " << bestMove.ToString() << std::endl;
        bestMove.Make(isBlack);
        std::cout << "Time taken: " << ms_nano.count()/1e6 << " ms" << std::endl;
        std::cout << std::endl;
        std::cout << boardState.GetStringRepresentation();
        cancelCalceler = true;
        canceler.join();

        std::string responseMove;
        std::cout << "Response: ";
        std::cin >> responseMove;
        StringToMove(responseMove).Make(!isBlack);
        std::cout << std::endl;
        std::cout << boardState.GetStringRepresentation();
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Hello World!\n";
#ifdef DEBUG
    bool isBlacksTurn = CreateFromFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", boardState);
    //bool isBlacksTurn = false;
    //Move moves[] = {
    //    StringToMove("h2h3"),
    //    StringToMove("a6a5"),
    //    StringToMove("g1h2"),
    //    StringToMove("a5a4"),
    //};
    //for (Move move : moves) {
    //    move.Make(isBlacksTurn);
    //    max_depth--;
    //    isBlacksTurn = !isBlacksTurn;
    //}
    //std::cout << boardState.GetStringRepresentation();
    if (isBlacksTurn) {
        std::cout << explorePerft<true>(max_depth) << std::endl;
    }
    else {
        std::cout << explorePerft<false>(max_depth) << std::endl;
    }
#else
    //long long timeTotal = 0;
    //for (int i = 0; i<10; i++) {
    //    auto t1 = std::chrono::high_resolution_clock::now();
    //    std::cout << perft<false>(5) << " nodes" << std::endl;
    //    auto t2 = std::chrono::high_resolution_clock::now();

    //    auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
    //    timeTotal += ms_nano.count();

    //    std::cout << i << ": " << ms_nano.count()/1e6 << " ms" << std::endl;
    //}
    //std::cout << timeTotal/1e6/10 << " ms" << std::endl;
    std::cout << "Updating attack and defend sets... ";
    UpdateAttackAndDefendSets<false>();
    UpdateAttackAndDefendSets<true>();
    std::cout << "Done!" << std::endl;
    play();

#endif // DEBUG

    return 0;
}

