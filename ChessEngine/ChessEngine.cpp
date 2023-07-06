// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <sstream>
#include <chrono>
#include <format>
#include "MoveGen.h"

Move movesPlayed[10];
Move movesUnplayed[10];
constexpr bool Debug = true;
u8 depth = 6;

char PromotionPiece(PieceType type) {
    switch (type)
    {
    case piece_type::KNIGHT:
        return 'N';
    case piece_type::BISHOP:
        return 'B';
    case piece_type::ROOK:
        return 'R';
    case piece_type::QUEEN:
        return 'Q';
    default:
        return '-';
        break;
    }
}

std::string MoveToString(Move move) {
    Location from = move.from;
    Location to = move.to;
    u8 fromFile = fileOf(from);
    u8 fromRank = rankOf(from);
    u8 toFile = fileOf(to);
    u8 toRank = rankOf(to);

    char promotionPiece = PromotionPiece(move.metadata & 7);

    return std::format("{:c}{:d} {:c}{:d} {:c}\n", fromFile + 'A', fromRank + 1, toFile + 'A', toRank + 1, promotionPiece);
}

std::string PlayedMovesToString() {
    std::string allMoves;
    for (int i = 0; i<depth; i++) {
        Move move = movesPlayed[i];

        allMoves += std::format("{{ {:2o}, {:2o}, {:2b} }}, \n", move.from, move.to, move.metadata);
    }
    allMoves += "\n";
    for (int i = 0; i<depth; i++) {
        Move move = movesPlayed[i];

        allMoves += MoveToString(move);
    }
    return allMoves;
}

template <bool Black>
size_t perftDebug(u8 depth, u8 max_depth) {
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
            std::cout << MoveToString(moves[i]);
        }
        else if (depth == max_depth-1) {
            std::cout << "  " << MoveToString(moves[i]);
        }
        movesPlayed[max_depth-depth] = moves[i];
        moves[i].Make<Black>();
        moves.push(movesLength);
        count += perftDebug<!Black>(depth - 1, max_depth);
        moves.pop(movesLength);
        movesUnplayed[max_depth-depth] = moves[i];
        moves[i].Unmake<Black>();

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

int main()
{
    std::cout << "Hello World!\n";
    if constexpr (Debug) {
        bool isBlacksTurn = false;
        //Move moves[] = {
        //    { 013, 023,  0 },
        //    { 062, 052,  0 },
        //    { 004, 013,  0 },
        //    { 073, 040,  0 },
        //    { 013, 004,  0 },
        //    { 040, 013,  0 },
        //};
        //for (Move move : moves) {
        //    move.Make(isBlacksTurn);
        //    depth--;
        //    isBlacksTurn = !isBlacksTurn;
        //}
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

