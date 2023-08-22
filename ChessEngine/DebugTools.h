#pragma once

#define DEBUG

#include <iostream>
#include <sstream>
#include <chrono>
#include "MoveGen.h"

u8 max_depth = 5;

template <bool Black>
size_t perft(u8 depth) {
    if (depth == 0) {
        return 1;
    }
    u16 movesLength = GenerateMoves<Black>();
    size_t count = 0;
    State prevState = boardState.state;
    for (int i = 0; i<movesLength; i++) {
        moves[i].Make<Black>();
        moves.push(movesLength);
        count += perft<!Black>(depth - 1);
        moves.pop();
        moves[i].Unmake<Black>(prevState);
    }
    return count;
}

inline Move StringToMove(std::string moveStr) {
    if (moveStr.size() != 4 && moveStr.size() != 5) throw std::invalid_argument("Size of string is not 4 or 5");
    Location from = (moveStr[0] - 'a') + 8*(moveStr[1] - '1');
    Location to = (moveStr[2] - 'a') + 8*(moveStr[3] - '1');
    if (moveStr.size() == 5) {
        switch (moveStr[4]) {
        case 'n': return Move::Make<PROMOTION, piece_type::KNIGHT>(from, to);
        case 'b': return Move::Make<PROMOTION, piece_type::BISHOP>(from, to);
        case 'r': return Move::Make<PROMOTION, piece_type::ROOK>(from, to);
        case 'q': return Move::Make<PROMOTION, piece_type::QUEEN>(from, to);
        case 'e': return Move::Make<EN_PASSANT>(from, to);
        default: throw std::invalid_argument("invalid 5th character");
        }
    }
    //if pawn
    if (uncoloredType(boardState.squares[from]) == piece_type::PAWN) {
        // if capture
        if (fileOf(from) - fileOf(to) != 0) {
            // if target empty
            if (boardState.squares[to] == 0) {
                return Move::Make<EN_PASSANT>(from, to);
            }
        }
    }
    return Move::Make(from, to);
}

template <class T, int N>
struct ConstexprArray {
    int size;
    T arr[N];
};

template <int Size = 100>
inline constexpr ConstexprArray<Move, Size> StringToMoves(std::string movesStr) {
    int index = 0;
    ConstexprArray<Move, Size> moves = {};
    std::string currMoveStr = "";
    for (size_t i = 0; i<movesStr.size(); i++) {
        if (movesStr[i] == ' ') {
            moves.arr[index] = StringToMove(currMoveStr);
            index++;
            currMoveStr = "";
        }
        else {
            currMoveStr += movesStr[i];
        }
    }
    if (currMoveStr != "") {
        moves.arr[index] = StringToMove(currMoveStr);
        index++;
        currMoveStr = "";
    }
    moves.size = index;
    return moves;
}

inline Move movePlayed[10];
inline Move moveUnplayed[10];

inline std::string PlayedMovesToString() {
    std::string allMoves;
    allMoves += "\n";
    for (int i = 0; i<max_depth; i++) {
        Move move = movePlayed[i];
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

    movePlayed[depth] = {};
    moveUnplayed[depth] = {};

    BoardState prevState = boardState;
    if (!(boardState == prevState)) throw "aaaaaa";
    for (int i = 0; i<movesLength; i++) {
        if (moves[i].To() > 63 || moves[i].From() > 63) {
            throw "Move out of bounds";
        }
        try {
            movePlayed[depth] = moves[i];
            moves[i].Make<Black>();
            moves.push(movesLength);
            size_t current = perftDebug<!Black>(depth - 1);
            count += current;
            moves.pop();
            moveUnplayed[depth] = moves[i];
            moves[i].Unmake<Black>(prevState.state);
        }
        catch (std::invalid_argument err) {
            boardState = {};
        }

        while (!(boardState == prevState)) {
            std::cout << boardState.Difference(prevState);
            try {
                boardState = prevState;
                GenerateMoves<Black>();
                std::cout << boardState.GetStringRepresentation() << moves[i].ToString() << std::endl;
                moves[i].Make<Black>();
                std::cout << boardState.GetStringRepresentation();
                moves.push(movesLength);
                count += perftDebug<!Black>(depth - 1);
                moves.pop();
                moves[i].Unmake<Black>(prevState.state);
                std::cout << boardState.GetStringRepresentation();
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
    int index = 0;
    std::cout << "Explore index: ";
    std::cin >> index;

    if (index == -1) return true;
    if (index == -2) return false;

    std::cout << "Chosen index " << index << ": " << moves[index].ToString() << std::endl;
    State prevState = boardState.state;
    moves[index].Make<Black>();
    moves.push(movesLength);
    bool reset = exploreMove<!Black>();
    moves.pop();
    moves[index].Unmake<Black>(prevState);

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
        State prevState = boardState.state;
        for (int i = 0; i<movesLength; i++) {
            std::cout << std::format("{:3d}", i) << ": " << moves[i].ToString() << ": ";
            moves[i].Make<Black>();
            moves.push(movesLength);
            size_t current = perftDebug<!Black>(depth - 1);
            count += current;
            std::cout << current << std::endl;
            moves.pop();
            moves[i].Unmake<Black>(prevState);
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
        moves.pop();
        moves[index].Unmake<Black>(prevState);
        if (exit) break;
    }

    return true;
}
