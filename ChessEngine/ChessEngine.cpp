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
            for (int times = 0; (times < 1*10) && !cancelCalceler; times++) {
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

bool playMoves(bool isBlacksTurn) {
    auto moves = StringToMoves<200>("e2e4 c7c5 d1f3 b8c6 f3c3 d7d5 c3c5 e7e6 c5e3 d5d4 e3b3 g8f6 e4e5 f6d5 g1f3 h7h5 h2h4 f8c5 b3c4 b7b6 d2d3 a7a5 c4b5 d8d7 b5b3 d7b7 b1d2 a5a4 b3b5 a8a5 b5c4 c5f8 a2a3 b6b5 c4a2 b5b4 a3b4 d5b4 a2b1 b7c7 d2c4 a5e5 c4e5 c6e5 f3d4 a4a3 a1a3 e5g4 f1e2 c7e5 a3a8 e5d4 e2g4 d4c5 g4d1 g7g5 c1g5 c5c6 a8c8 c6c8 c2c3 c8a6 c3b4 f8d6 g5d2 h8g8 d1h5 e8e7 e1g1 a6b5 b1d1 b5e5 g2g3 g8g3 g1h2 g3g8 f2f4 e5g7 d1f3 d6c7 f1c1 c7f4 d2f4 g7b2 h2h3 g8f8 f4h6 b2c1 h6c1 e6e5 c1e3 e7d7 f3d5 d7e7");
    for (int i = 0; i<moves.size; i++) {
        std::cout << moves.arr[i].ToString() << std::endl;
        moves.arr[i].Make(isBlacksTurn);
        isBlacksTurn = !isBlacksTurn;
    }
    return isBlacksTurn;
}

int main()
{
    std::cout << "Hello World!\n";
#ifdef DEBUG
    bool isBlacksTurn = false; // CreateFromFEN("5r2/4kp2/8/3Qp2B/1P5P/3PB2K/8/8 w - -", boardState);
    isBlacksTurn = playMoves(isBlacksTurn);
    //bool isBlacksTurn = false;
    Search(isBlacksTurn);
    //std::cout << boardState.GetStringRepresentation();
    //if (isBlacksTurn) {
    //    std::cout << explorePerft<true>(max_depth) << std::endl;
    //}
    //else {
    //    std::cout << explorePerft<false>(max_depth) << std::endl;
    //}
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

