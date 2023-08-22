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

void playSelf(int argc, char *argv[]) {
    bool turn = false;
    if (argc == 2) {
        std::cout << "Creating from fen " << argv[1] << std::endl;
        turn = CreateFromFEN(argv[1], boardState);
        std::cout << boardState.GetStringRepresentation();
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

        Move bestMove = Search(turn);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        std::cout << "Best move: " << bestMove.ToString() << std::endl;
        bestMove.Make(turn);
        std::cout << "Time taken: " << ms_nano.count()/1e6 << " ms" << std::endl;
        std::cout << std::endl;
        std::cout << boardState.GetStringRepresentation();
        cancelCalceler = true;
        canceler.join();

        turn = !turn;

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

        hashedPositions.clear();

        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        timeTotal += ms_nano.count();

        std::cout << std::setfill(' ') << std::setw(3) << i << ": " << ms_nano.count()/1e6 << " ms\n";
    }
    std::cout << timeTotal/1e6/N << " ms" << std::endl;
}

void TestMoves(int argc, char *argv[]) {
    bool isBlacksTurn = false; // CreateFromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", boardState);
    //max_depth = 6;
    playMoves(isBlacksTurn, "d2d4 e7e6 e2e4 b8c6 f2f4 d7d5 e4e5 d8h4 g2g3 h4d8 c2c3 c8d7 d1b3 c6a5 b3c2 a5c6 f1d3 h7h5 g1f3 a7a5 e1g1 f8e7 c2g2 g8h6 h2h3 h6f5 f1e1 g7g6 a2a4 b7b6 b2b3 a8a7 b1d2 f5e3 e1e3 e8g8 a1a2 g8h8 d2f1 f8g8 e3e2 d8b8 d3b5 b8b7 c3c4 c6b4 b5d7 b4a2 c1e3 c7c6 e2a2 b7d7 g3g4 h5h4 g2e2 d7c7 a2b2 g8d8 e2f2 d5c4 b3c4 h8g7 f4f5 g6f5 f1h2");
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
    if (isBlacksTurn) {
        std::cout << explorePerft<true>(max_depth) << std::endl;
    }
    else {
        std::cout << explorePerft<false>(max_depth) << std::endl;
    }
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
    //CreateFromFEN("8/7k/Q4Q2/2N5/8/8/8/K6 w - -", boardState);

    //Move bestMove = Search<false>();
    //std::cout << "Best move: " << bestMove.ToString() << std::endl;
    //play(argc, argv);
    //playSelf(argc, argv);
    BenchmarkSearch();
    //BenchmarkPerft();
    //perft<false>(6);
    //Search<false>();
    //TestMoves(argc, argv);
    //std::cout << std::hardware_constructive_interference_size;

    return 0;
}

