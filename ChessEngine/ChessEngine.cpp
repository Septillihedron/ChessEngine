// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <thread>
#include <iomanip>
#include "DebugTools.h"
#include "AlphaBeta.h"

void playMoves(bool &isBlacksTurn, std::string movesStr) {
    auto moves = StringToMoves<100>(movesStr);
    for (int i = 0; i<moves.size; i++) {
        std::cout << moves.arr[i].ToString() << std::endl;
        moves.arr[i].Make(isBlacksTurn);
        isBlacksTurn = !isBlacksTurn;
        max_depth--;
    }
    boardState.RemoveHistory();
}

void play(int argc, char *argv[]) {
    bool isBlack = false;
    char isBlackChar;
    std::cout << "Turn: ";
    std::cin >> isBlackChar;
    isBlack = isBlackChar == 'b';
    bool turn = isBlack;
    if (argc == 2) {
        std::cout << "Creating from fen " << argv[1] << std::endl;
        turn = CreateFromFEN(argv[1], boardState);
        std::cout << boardState.GetStringRepresentation();
    }
    if (turn != isBlack) {
        std::string responseMove;
        std::cout << "Response: ";
        std::cin >> responseMove;
        StringToMove(responseMove).Make(!turn);
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
        //std::cout << boardState.GetStringRepresentation();
        cancelCalceler = true;
        canceler.join();

        std::string responseMove;
        std::cout << "Response: ";
        std::cin >> responseMove;
        StringToMove(responseMove).Make(!isBlack);
        std::cout << std::endl;
        //std::cout << boardState.GetStringRepresentation();

        boardState.RemoveHistory();
    }
}

void BenchmarkPerft() {
    //CreateFromFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", boardState);
    long long timeTotal = 0;
    u8 maxDepth = 6;
    u8 N = 20;
    for (int i = 0; i<5; i++) {
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

        std::cout << std::setfill(' ') << std::setw(3) << i << ": " << ms_nano.count()/1e6 << " ms\n";
    }
    std::cout << timeTotal/1e6/N << " ms" << std::endl;
}

void BenchmarkSearch() {
    CreateFromFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", boardState);
    long long timeTotal = 0;
    u8 N = 20;
    for (int i = 0; i<N; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        Search<false>();
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        timeTotal += ms_nano.count();

        std::cout << std::setfill(' ') << std::setw(3) << i << ": " << ms_nano.count()/1e6 << " ms\n";
    }
    std::cout << timeTotal/1e6/N << " ms" << std::endl;
}


int main(int argc, char *argv[])
{
    std::cout << "Hello World!\n";
    movesPlayed.reserve(100);
    // add padding
    for (Location i = 0; i<12; i++) {
        Move m = Move::Make(i, i);
        movesPlayed.push_back(m);
    }
#ifdef DEBUG
    bool isBlacksTurn = false; // CreateFromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", boardState);
    //max_depth = 6;
    //playMoves(isBlacksTurn, "b4c4 d6d5 a5b4 c7c5");
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
    //CreateFromFEN("8/7k/Q4Q2/2N5/8/8/8/K6 w - -", boardState);

    //Move bestMove = Search<false>();
    //std::cout << "Best move: " << bestMove.ToString() << std::endl;
    //play(argc, argv);
    //BenchmarkSearch();
    //BenchmarkPerft();
    //perft<false>(6);
    //Search<false>();
    std::cout << std::hardware_constructive_interference_size;

#endif // DEBUG

    return 0;
}

