// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <thread>
#include "DebugTools.h"
#include "AlphaBeta.h"

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
    bool isBlacksTurn = false; // CreateFromFEN("1n1q1rk1/n4ppp/1b6/2P2Q2/8/4P3/PP3PPP/R1B1K1NR b KQ - 0 16", boardState);
    //bool isBlacksTurn = false;
    auto moves = StringToMoves("b1c3 c7c5 c3b5 d8a5 b5c7 a5c7 e2e3 d7d5 f1b5 c8d7 b5d7 b8d7 d1g4 e8c8 g4d7 d8d7 c2c3 e7e6 g2g3 g8f6 a1b1 f6e4 e1d1 e4f2 d1e2 f2h1 g1f3 c7a5 f3g1 a5b5");
    for (int i = 0; i<moves.size; i++) {
        Move move = moves.arr[i];
        std::cout << move.ToString() << std::endl;
        move.Make(isBlacksTurn);
        //max_depth--;
        isBlacksTurn = !isBlacksTurn;
        //std::cout << boardState.GetStringRepresentation();
        //std::cin.ignore();
    }
    Search(isBlacksTurn);
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
    play();

#endif // DEBUG

    return 0;
}

