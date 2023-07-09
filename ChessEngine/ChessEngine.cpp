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
        auto cancelFunction = []() {
            std::this_thread::sleep_for(std::chrono::seconds(29));
            searchCanceled = true;
        };
        std::thread canceler(cancelFunction);

        auto t1 = std::chrono::high_resolution_clock::now();

        Move bestMove = Search(isBlack);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        std::cout << "Best move: "<< bestMove.ToString() << std::endl;
        bestMove.Make(isBlack);
        std::cout << "Time taken: "<< ms_nano.count()/1e6 << " ms" << std::endl;
        std::cout << std::endl;
        std::cout << boardState.GetStringRepresentation();
        canceler.join();

        std::string responseMove;
        std::cout << "Response: ";
        std::cin >> responseMove;
        StringToMove(responseMove).Make(!isBlack);
        std::cout << std::endl;
        std::cout << boardState.GetStringRepresentation();
    }
}

int main()
{
    std::cout << "Hello World!\n";
#ifdef DEBUG
    //bool isBlacksTurn = CreateFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", boardState);
    //bool isBlacksTurn = CreateFromFEN("8/8/8/8/R4pk1/8/4PK2/8 w - -", boardState);
    bool isBlacksTurn = false;
    //Move moves[] = {
    //    StringToMove("e2e4")
    //};
    //for (Move move : moves) {
    //    move.Make(isBlacksTurn);
    //    max_depth--;
    //    isBlacksTurn = !isBlacksTurn;
    //}
    std::cout << boardState.GetStringRepresentation();
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

