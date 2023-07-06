// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <sstream>
#include <chrono>
#include "MoveGen.h"


void PrintMove(Move move) {
    Location from = move.from;
    Location to = move.to;
    u8 fromFile = fileOf(from);
    u8 fromRank = rankOf(from);
    u8 toFile = fileOf(to);
    u8 toRank = rankOf(to);

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

    printf("%c%d%c%d%c\n", fromFile + 'A', (int) fromRank + 1, toFile + 'A', (int) toRank + 1, promotionPiece);
    printf("0%d%d, 0%d%d, %c\n", fromRank, fromFile, toRank, toFile, promotionPiece);
    
}

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

    movesPlayed[max_depth-depth] = {};
    movesUnplayed[max_depth-depth] = {};

    BoardState prevState = boardState;
    for (int i = 0; i<movesLength; i++) {
        if (moves[i].to > 63 || moves[i].from > 63) {
            throw "Move out of bounds";
        }

        if (depth == max_depth) {
            PrintMove(moves[i]);
        }

        moves[i].Make<Black>();
        movesPlayed[max_depth-depth] = moves[i];
        moves.push(movesLength);
        count += perftDebug<!Black>(depth - 1, max_depth);
        moves.pop(movesLength);
        moves[i].Unmake<Black>();
        movesUnplayed[max_depth-depth] = moves[i];

        while (!(boardState == prevState)) {
            std::cout << boardState.Difference(prevState);
            try {
                boardState = prevState;
                moves[i].Make<Black>();
                moves.push(movesLength);
                count += perftDebug<!Black>(depth - 1, max_depth);
                moves.pop(movesLength);
                moves[i].Unmake<Black>();
            }
            catch (std::invalid_argument err) {

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

constexpr bool Debug = true;
u8 depth = 6;

int main()
{
    std::cout << "Hello World!\n";
    if constexpr (Debug) {
        Move moves[] = {
            { 006, 025, 0 },
            { 063, 053, 0 },
            { 025, 044, 0 },
        };
        bool isBlacksTurn = false;
        for (Move move : moves) {
            move.Make(isBlacksTurn);
            depth--;
            isBlacksTurn = !isBlacksTurn;
        }
        if (isBlacksTurn) {
            std::cout << perftDebug<true>(depth, depth) << std::endl;
        }
        else {
            std::cout << perftDebug<false>(depth, depth) << std::endl;
        }
    }
    if constexpr (!Debug) {
        long long timeTotal = 0;
        for (int i = 0; i<100; i++) {
            auto t1 = std::chrono::high_resolution_clock::now();
            std::cout << perft<false>(depth, depth) << std::endl;
            auto t2 = std::chrono::high_resolution_clock::now();

            auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
            timeTotal += ms_nano.count();

            std::cout << i << ": " << ms_nano.count() << " nanoseconds" << std::endl;
        }
        std::cout << timeTotal/1e6/100.0 << std::endl;
    }
    return 0;
}

