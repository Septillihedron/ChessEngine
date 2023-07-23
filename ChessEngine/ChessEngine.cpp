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

void playMoves(bool &isBlacksTurn, std::string movesStr) {
    auto moves = StringToMoves<100>(movesStr);
    for (int i = 0; i<moves.size; i++) {
        std::cout << moves.arr[i].ToString() << std::endl;
        moves.arr[i].Make(isBlacksTurn);
        isBlacksTurn = !isBlacksTurn;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Hello World!\n";
#ifdef DEBUG
    bool isBlacksTurn = false; // CreateFromFEN("5r2/4kp2/8/3Qp2B/1P5P/3PB2K/8/8 w - -", boardState);
    //playMoves(isBlacksTurn, "b1c3 g8f6 f2f3 f6e4");
    if (argc >= 2) {
        std::string fenString = argv[1];
        if (fenString != "startpos") {
            isBlacksTurn = CreateFromFEN(fenString, boardState);
        }
    }
    if (argc >= 3) {
        max_depth = (argv[2][0] - '0');
    }
    //std::cout << boardState.GetStringRepresentation() << max_depth << std::endl;
    //isBlacksTurn = playMoves(isBlacksTurn);
    //bool isBlacksTurn = false;
    //Search(isBlacksTurn);
    //std::cout << boardState.GetStringRepresentation();
    if (isBlacksTurn) {
        std::cout << explorePerft<true>(max_depth) << std::endl;
    }
    else {
        std::cout << explorePerft<false>(max_depth) << std::endl;
    }
#else
    CreateFromFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", boardState);
    long long timeTotal = 0;
    u8 maxDepth = 4;
    u8 N = 100;
    for (int i = 0; i<10; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        perft<false>(maxDepth);
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);

        std::cout << i << ": " << ms_nano.count()/1e6 << " ms" << std::endl;
    }
    std::cout << "Warm up finished" << std::endl;
    for (int i = 0; i<N; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        perft<false>(maxDepth);
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        timeTotal += ms_nano.count();

        std::cout << std::format("{:3d}: {:f} ms", i, ms_nano.count()/1e6) << std::endl;
    }
    std::cout << timeTotal/1e6/N << " ms" << std::endl;
    //play();

#endif // DEBUG

    return 0;
}

