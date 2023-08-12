@echo off

g++ -std=c++23 -Ofast -Wall -march=native -flto -fprofile-generate ChessEngine\*.cpp -o ChessEngineGpp.exe
ChessEngineGpp.exe
g++ -std=c++23 -Ofast -Wall -march=native -flto -fprofile-use ChessEngine\*.cpp -o ChessEngineGpp.exe
ChessEngineGpp.exe
