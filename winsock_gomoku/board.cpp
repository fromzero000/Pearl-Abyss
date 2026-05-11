#include "board.h"
#include <iomanip>
#include <sstream>
#include <queue>
Board::Board() : grid(SIZE, std::vector<Stone>(SIZE, Stone::Empty)) {}

bool Board::isValidPosition(int r, int c) const
{
    return (r >= 0 && r < SIZE && c >= 0 && c < SIZE);
}

bool Board::placeStone(int r, int c, Stone stone)
{
    if (!isValidPosition(r, c))
    {
        std::cout << "[Warning] Invalid Position\n";
        std::string pause;
        std::cin.ignore();
        std::getline(std::cin, pause);
        return false;
    }

    if (grid[r][c] != Stone::Empty)
    {
        std::cout << "[Warning] Already Placed\n";
        return false;
    }

    grid[r][c] = stone;
    return true;
}

void Board::printBoard() const
{
    std::cout << "   ";
    for (int i = 1; i <= SIZE; i++)
        std::cout << std::setw(2) << i << ' ';
    std::cout << '\n';

    for (int i = 0; i < SIZE; i++)
    {
        std::cout << std::setw(2) << i + 1 << ' ';
        for (int j = 0; j < SIZE; j++)
        {
            char stone;
            switch (grid[i][j])
            {
            case Stone::Empty:
                stone = '.';
                break;
            case Stone::Black:
                stone = 'O';
                break;
            default:
                stone = 'X';
                break;
            }
            std::cout << std::setw(2) << stone << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}
enum class Direction
{
    Horizontal = 0,
    Vertical,
    Decreasing,
    Increasing 
};


bool Board::checkWin(int r, int c, Stone stone) const
{
    //check horizontal
    int cnt_hor = 1;
    for (int i = r-1; i >= 0; i--)
    {
        if (grid[i][c] != stone) break;
        else cnt_hor++;
    }

    for (int i = r + 1; i < SIZE; i++)
    {
        if (grid[i][c] != stone) break;
        else cnt_hor++;
    }

    if ((stone == Stone::Black && cnt_hor == 5) || (stone == Stone::White && cnt_hor >= 5)) return true;

    //check vertical
    int cnt_ver = 1;
    for (int i = c - 1; i >= 0; i--)
    {
        if (grid[r][i] != stone) break;
        else cnt_ver++;
    }

    for (int i = c + 1; i < SIZE; i++)
    {
        if (grid[r][i] != stone) break;
        else cnt_ver++;
    }
    
    if ((stone == Stone::Black && cnt_ver == 5) || (stone == Stone::White && cnt_ver >= 5)) return true;

    //check decreasing diagornal
    int cnt_dec = 1;
    int nr = r - 1, nc = c - 1;
    while (nr >= 0 && nc >= 0)
    {
        if (grid[nr][nc] != stone)
        {
            break;
        }
        cnt_dec++;
        nr--;
        nc--;
    }

    nr = r + 1, nc = c + 1;
    while (nr < SIZE && nc < SIZE)
    {
        if (grid[nr][nc] != stone)
        {
            break;
        }
        cnt_dec++;
        nr++;
        nc++;
    }

    if ((stone == Stone::Black && cnt_dec == 5) || (stone == Stone::White && cnt_dec >= 5)) return true;

    //check Increasing diagornal
    int cnt_inc = 1;
    nr = r + 1, nc = c - 1;
    while (nr < SIZE && nc >= 0)
    {
        if (grid[nr][nc] != stone)
        {
            break;
        }
        cnt_inc++;
        nr++;
        nc--;
    }

    nr = r - 1, nc = c + 1;
    while (nr >= 0 && nc < SIZE)
    {
        if (grid[nr][nc] != stone)
        {
            break;
        }
        cnt_inc++;
        nr--;
        nc++;
    }

    if ((stone == Stone::Black && cnt_inc == 5) || (stone == Stone::White && cnt_inc >= 5)) return true;

    return false;
}
