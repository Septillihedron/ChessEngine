// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <sstream>
#include <chrono>
#include "MoveGen.h"


void PrintMove(Move move) {
    Location from = move.from;
    Location to = move.to;
    char fromFile = fileOf(from) + 'A';
    int fromRank = (int) rankOf(from) + 1;
    char toFile = fileOf(to) + 'A';
    int toRank = (int) rankOf(to) + 1;

    char promotionPiece = '-';

    switch (move.metadata)
    {
    case piece_type::KNIGHT:
        promotionPiece = 'N';
        break;
    case piece_type::BISHOP:
        promotionPiece = 'B';
        break;
    case piece_type::ROOK:
        promotionPiece = 'R';
        break;
    case piece_type::QUEEN:
        promotionPiece = 'Q';
        break;
    default:
        break;
    }

    std::cout << fromFile << fromRank << toFile << toRank << promotionPiece << std::endl;
}

constexpr bool Debug = true;

Move movesPlayed[10];
Move movesUnplayed[10];

template <bool Black>
size_t perftDebug(u8 depth, u8 max_depth) {
    BoardState generatedFromState = boardState;
    size_t movesLength = GenerateMoves<Black>();
    if (depth == 0) {
        return movesLength;
    }
    size_t count = 0;
    if constexpr (Debug) {
        movesPlayed[max_depth-depth] = {};
        movesUnplayed[max_depth-depth] = {};
    }
    BoardState prevState = boardState;
    for (int i = 0; i<movesLength; i++) {
        if constexpr (Debug) {
            if (moves[i].to > 63 || moves[i].from > 63) {
                throw "Move out of bounds";
            }
        }

        moves[i].Make();
        if constexpr (Debug) movesPlayed[max_depth-depth] = moves[i];
        moves.push(movesLength);
        count += perft<!Black>(depth - 1, max_depth);
        moves.pop(movesLength);
        moves[i].Unmake();
        if constexpr (Debug) movesUnplayed[max_depth-depth] = moves[i];
        if constexpr (Debug) {
            while (!(boardState == prevState)) {
                try {
                    boardState = prevState;
                    moves[i].Make();
                    moves.push(movesLength);
                    count += perft<!Black>(depth - 1, max_depth);
                    moves.pop(movesLength);
                    moves[i].Unmake();
                }
                catch (std::invalid_argument err) {

                }
            }
        }
    }
    return count;
}

template <bool Black>
size_t perft(u8 depth, u8 max_depth) {
    size_t movesLength = GenerateMoves<Black>();
    if (depth == 0) {
        return movesLength;
    }
    size_t count = 0;
    for (int i = 0; i<movesLength; i++) {
        moves[i].Make();
        moves.push(movesLength);
        count += perft<!Black>(depth - 1, max_depth);
        moves.pop(movesLength);
        moves[i].Unmake();
    }
    return count;
}

int main()
{
    std::cout << "Hello World!\n";

    for (int i = 0; i<10; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << perft<false>(6, 6) << std::endl;
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

        std::cout << i << ": " << ms_int.count() << " ms" << std::endl;
    }
    return 0;
}

