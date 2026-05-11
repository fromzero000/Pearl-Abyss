#include "board.h"
#include <iostream>

int main()
{
	Board gameBoard;
	int row, col;
	Stone currentTurn = Stone::Black;
	while (1)
	{
		gameBoard.printBoard();		
		if (currentTurn == Stone::Black) std::cout << "Black(O) Turn";
		else std::cout << "White(X) Turn";
		std::cout << "\nInput [row col] (-1 -1 to quit) : ";
		std::cin >> row >> col;

		if (row == -1 || col == -1) break;

		if (gameBoard.placeStone(row - 1, col - 1, currentTurn))
		{
			if (gameBoard.checkWin(row - 1, col - 1, currentTurn))
			{
				break;
			}
			currentTurn = (currentTurn == Stone::Black) ? Stone::White : Stone::Black;
		}
		system("cls");
	}
	system("cls");
	gameBoard.printBoard();
	std::cout
		<< (currentTurn == Stone::Black ? "Black" : "White")
		<< " Win!!!\n";
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.get();
}