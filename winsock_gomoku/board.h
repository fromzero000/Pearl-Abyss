#pragma once
#include <vector>
#include <iostream>

struct MoveData
{
	int row, col;
};

enum class Stone {
	Empty = 0,
	Black,
	White
};

class Board {
	static constexpr int SIZE = 15;
	std::vector<std::vector<Stone>> grid;
public:
	Board();

	void printBoard() const;
	bool placeStone(int r, int c, Stone stone);
	bool checkWin(int r, int c, Stone stone) const;

	bool isValidPosition(int r, int c) const;
};