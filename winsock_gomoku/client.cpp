#include <iostream>
#include <vector>
#include <sstream>
#include <limits>

#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../common/board.h"
#pragma comment(lib, "ws2_32.lib")

void ErrorHandling(const std::string& message, SOCKET hServSock = INVALID_SOCKET);

int main(int argc, char *argv[])
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error");

    SOCKET hServSock;
    sockaddr_in servAddr{};
    
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET)
        ErrorHandling("socket() error");

    servAddr.sin_family = AF_INET;
    int inetPtonResult = inet_pton(AF_INET, argv[1], &servAddr.sin_addr);
    //visual studio에서 inetPton 사용시 유니코드 관련 에러 발생
    if (inetPtonResult != 1)
    {
        if (inetPtonResult == 0)
            ErrorHandling("Invalid IP Address", hServSock);
        else
        {
            int errCode = WSAGetLastError();
            ErrorHandling("inet_pton() error, error Code : "+std::to_string(errCode), hServSock);
        }
    }
    servAddr.sin_port = htons(atoi(argv[2]));

    if (connect(hServSock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        ErrorHandling("connect() error", hServSock);
    else
        std::cout << "Connected...\n";

    /*Game Init*/
    Board gameBoard{};
    int row{}, col{};
    Stone myStone = Stone::White;
    Stone peerStone = Stone::Black;
    system("cls");
    gameBoard.printBoard();
    std::cout << "Game Start!!\n";
    /*Game Loop*/
    while (1)
    {
        
        MoveData peerMove;
        int receivedDataLen = recv(hServSock, (char*)&peerMove, sizeof(MoveData), 0);
        if (receivedDataLen <= 0)
        {
            std::cout << "Server Disconnected...\n";
            break;
        }

        int peerRow = ntohl(peerMove.row);
        int peerCol = ntohl(peerMove.col);

        gameBoard.placeStone(peerRow - 1, peerCol - 1, peerStone);

        system("cls");
        gameBoard.printBoard();

        if (gameBoard.checkWin(peerRow - 1, peerCol - 1, peerStone))
        {
            std::cout << "*** Black(O) Win!!! ***\n";
            break;
        }
        
        std::cout << "My Turn\nInput [row col] (-1 -1 to quit) : ";
        while (1)
        {
            std::cin >> row >> col;

            if (row == -1 || col == -1) break;

            if (gameBoard.placeStone(row - 1, col - 1, myStone))
            {
                break;
            }
        }
        system("cls");
        gameBoard.printBoard();

        

        //Sending my move
        MoveData myMove;
        myMove.row = htonl(row);
        myMove.col = htonl(col);
        send(hServSock, (char*)&myMove, sizeof(MoveData), 0);
        if (gameBoard.checkWin(row - 1, col - 1, myStone))
        {
            std::cout << "White(X) Win!!!";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            break;
        }
    }
    closesocket(hServSock);
    WSACleanup();
    return 0;
}

void ErrorHandling(const std::string& message, SOCKET hServSock)
{
    std::cerr << message << '\n';
    if (hServSock != INVALID_SOCKET) closesocket(hServSock);
    WSACleanup();
    exit(1);
}