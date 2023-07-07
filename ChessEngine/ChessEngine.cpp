// ChessEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.

#define DEBUG

#include <iostream>
#include <sstream>
#include <chrono>
#include "MoveGen.h"

u8 depth = 5;

#ifdef DEBUG
#include <format>
Move movesPlayed[10];
Move movesUnplayed[10];

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

    return std::format("{:c}{:d} {:c}{:d} {:c}", fromFile + 'A', fromRank + 1, toFile + 'A', toRank + 1, promotionPiece);
}
std::string MoveInitiatorString(Move move) {
    return std::format("{{ {:#03o}, {:#03o}, {:#03b} }}, ", move.from, move.to, move.metadata);
}

std::string PlayedMovesToString() {
    std::string allMoves;
    for (int i = 0; i<depth; i++) {
        Move move = movesPlayed[i];
        allMoves += MoveInitiatorString(move);
    }
    allMoves += "\n";
    for (int i = 0; i<depth; i++) {
        Move move = movesPlayed[i];
        allMoves += MoveToString(move);
    }
    return allMoves;
}

bool MovesArrayEq(Move *a, Move *b, int n) {
    for (int i = 0; i<n; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
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
            std::cout << MoveToString(moves[i]) << std::endl;
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
bool exploreMove() {
    size_t movesLength = GenerateMoves<Black>();
    std::cout << movesLength << std::endl;
    for (int i = 0; i<movesLength; i++) {
        std::cout << std::format("{:3d}", i) << ": " << MoveToString(moves[i]) << " " << MoveInitiatorString(moves[i]) << std::endl;
    }
    int index = 0;
    std::cout << "Explore index: ";
    std::cin >> index;

    if (index == -1) return true;
    if (index == -2) return false;

    std::cout << "Chosen index " << index << ": " << MoveToString(moves[index]) << " " << MoveInitiatorString(moves[index]) << std::endl;
    moves[index].Make<Black>();
    moves.push(movesLength);
    bool reset = exploreMove<!Black>();
    moves.pop(movesLength);
    moves[index].Unmake<Black>();

    std::cout << MoveInitiatorString(moves[index]) << std::endl;
    return reset;
}

#endif // DEBUG

template <bool Black>
size_t perft(u8 depth, u8 max_depth) {
    size_t movesLength = GenerateMoves<Black>();
    if (depth == 0) {
        return movesLength;
    }
    size_t count = 0;
    for (int i = 0; i<movesLength; i++) {
        if (uncoloredType(boardState.squares[moves[i].to]) == piece_type::KING) continue;
        moves[i].Make<Black>();
        moves.push(movesLength);
        count += perft<!Black>(depth - 1, max_depth);
        moves.pop(movesLength);
        moves[i].Unmake<Black>();
    }
    return count;
}

int main()
{
    std::cout << "Hello World!\n";
#ifdef DEBUG
    //while (exploreMove<false>()) {}
    //if (true) return 0;
    bool isBlacksTurn = false;
    //Move moves[] = {
    //    { 012, 022,  0 }
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
#else
    long long timeTotal = 0;
    for (int i = 0; i<10; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << perft<false>(depth, depth) << " nodes" << std::endl;
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        timeTotal += ms_nano.count();

        std::cout << i << ": " << ms_nano.count()/1e6 << " ms" << std::endl;
    }
    std::cout << timeTotal/1e6/10 << " ms" << std::endl;
#endif // DEBUG

    return 0;
}

