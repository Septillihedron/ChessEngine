#pragma once

//#define DEBUG

#include <iostream>
#include <sstream>
#include <chrono>
#include <format>
#include "MoveGen.h"

u8 max_depth = 5;

template <bool Black>
size_t perft(u8 depth) {
    if (depth == 0) {
        return 1;
    }
    u16 movesLength = GenerateMoves<Black>();
    size_t count = 0;
    for (int i = 0; i<movesLength; i++) {
        moves[i].Make<Black>();
        moves.push(movesLength);
        count += perft<!Black>(depth - 1);
        moves.pop(movesLength);
        moves[i].Unmake<Black>();
    }
    return count;
}

inline char PromotionPiece(PieceType type) {
    switch (type)
    {
    case piece_type::KNIGHT:
        return 'n';
    case piece_type::BISHOP:
        return 'b';
    case piece_type::ROOK:
        return 'r';
    case piece_type::QUEEN:
        return 'q';
    default:
        return '-';
        break;
    }
}

inline constexpr Move StringToMove(std::string moveStr) {
    Move moveMove;
    if (moveStr.size() != 4 && moveStr.size() != 5) throw std::invalid_argument("Size of string is not 4 or 5");
    moveMove.from = (moveStr[0] - 'a') + 8*(moveStr[1] - '1');
    moveMove.to = (moveStr[2] - 'a') + 8*(moveStr[3] - '1');
    moveMove.metadata = 0;
    if (moveStr.size() == 5) {
        switch (moveStr[4]) {
        case 'n':
            moveMove.metadata = piece_type::KNIGHT;
            break;
        case 'b':
            moveMove.metadata = piece_type::BISHOP;
            break;
        case 'r':
            moveMove.metadata = piece_type::ROOK;
            break;
        case 'q':
            moveMove.metadata = piece_type::QUEEN;
            break;
        default:
            break;
        }
    }
    return moveMove;
}

#ifdef DEBUG
inline Move movesPlayed[10];
inline Move movesUnplayed[10];

inline std::string MoveInitiatorString(Move move) {
    return std::format("{{ {:#03o}, {:#03o}, {:#03b} }}, ", move.from, move.to, move.metadata);
}

inline std::string PlayedMovesToString() {
    std::string allMoves;
    for (int i = 0; i<max_depth; i++) {
        Move move = movesPlayed[i];
        allMoves += MoveInitiatorString(move);
    }
    allMoves += "\n";
    for (int i = 0; i<max_depth; i++) {
        Move move = movesPlayed[i];
        allMoves += move.ToString();
    }
    return allMoves;
}

inline bool MovesArrayEq(Move *a, Move *b, int n) {
    for (int i = 0; i<n; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

template <bool Black>
size_t perftDebug(u8 depth) {
    if (depth == 0) {
        return 1;
    }
    u8 movesLength = GenerateMoves<Black>();
    size_t count = 0;

    movesPlayed[max_depth-depth] = {};
    movesUnplayed[max_depth-depth] = {};

    BoardState prevState = boardState;
    for (int i = 0; i<movesLength; i++) {
        if (moves[i].to > 63 || moves[i].from > 63) {
            throw "Move out of bounds";
        }
        movesPlayed[max_depth-depth] = moves[i];
        moves[i].Make<Black>();
        moves.push(movesLength);
        size_t current = perftDebug<!Black>(depth - 1, max_depth);
        count += current;
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
    u8 movesLength = GenerateMoves<Black>();
    std::cout << movesLength << std::endl;
    for (int i = 0; i<movesLength; i++) {
        std::cout << std::format("{:3d}", i) << ": " << moves[i].ToString() << " " << MoveInitiatorString(moves[i]) << std::endl;
    }
    int index = 0;
    std::cout << "Explore index: ";
    std::cin >> index;

    if (index == -1) return true;
    if (index == -2) return false;

    std::cout << "Chosen index " << index << ": " << moves[index].ToString() << " " << MoveInitiatorString(moves[index]) << std::endl;
    moves[index].Make<Black>();
    moves.push(movesLength);
    bool reset = exploreMove<!Black>();
    moves.pop(movesLength);
    moves[index].Unmake<Black>();

    std::cout << MoveInitiatorString(moves[index]) << std::endl;
    return reset;
}
__forceinline
bool exploreMove(bool black) {
    if (black) {
        return exploreMove<true>();
    }
    else {
        return exploreMove<false>();
    }
}

template <bool Black>
bool explorePerft(u8 depth) {
    if (depth == 0) return false;
    while (depth <= max_depth) {
        u8 movesLength = GenerateMoves<Black>();
        size_t count = 0;
        //std::cout << movesLength << std::endl;
        for (int i = 0; i<movesLength; i++) {
            std::cout << std::format("{:3d}", i) << ": " << moves[i].ToString() << ": ";
            moves[i].Make<Black>();
            moves.push(movesLength);
            size_t current = perft<!Black>(depth - 1);
            count += current;
            std::cout << current << std::endl;
            moves.pop(movesLength);
            moves[i].Unmake<Black>();
        }
        std::cout << std::endl << "Nodes searched: " << count << std::endl;
        int index = 0;
        //std::cout << "Explore index: ";
        std::cin >> index;

        if (index == -1) continue;
        if (index == -2) return false;
        if (index == -3) return true;

        //std::cout << "Chosen index " << index << ": " << moves[index].ToString() << std::endl;
        moves[index].Make<Black>();
        moves.push(movesLength);
        bool exit = explorePerft<!Black>(depth - 1);
        moves.pop(movesLength);
        moves[index].Unmake<Black>(); 
        if (exit) break;
    }

    return true;
}

#endif // DEBUG