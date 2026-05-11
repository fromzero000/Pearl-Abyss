#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <limits>
#include "../common/board.h"


#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
void ErrorHandling(const std::string& message, SOCKET hServSock = INVALID_SOCKET, SOCKET hClntSock = INVALID_SOCKET);

int main(int argc, char* argv[])
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error");

    SOCKET hServSock, hClntSock;
    sockaddr_in servAddr{}, clntAddr{};
    int clntAddrSize;
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if(hServSock == INVALID_SOCKET)
        ErrorHandling("socket() error");
    
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (sockaddr *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");

    if(listen(hServSock, 1) == SOCKET_ERROR)
        ErrorHandling("listen() error");
    clntAddrSize = sizeof(clntAddr);
    hClntSock = accept(hServSock, (sockaddr*)&clntAddr, &clntAddrSize);
    if (hClntSock == -1)
        ErrorHandling("accept() error");
    else
        std::cout << "Connected client\n";


    /*Game Init*/
    Board gameBoard{};
    int row{}, col{};
    Stone myStone = Stone::Black;
    Stone peerStone = Stone::White;

    /*Game Loop*/
    while (1)
    {
        system("cls");
        gameBoard.printBoard();
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
        send(hClntSock, (char*)&myMove, sizeof(MoveData), 0);

        if (gameBoard.checkWin(row - 1, col - 1, myStone))
        {
            std::cout << "Black(O) Win!!!";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            break;
        }
        
        

        //Receive peer move
        MoveData peerMove;
        std::cout << "Waiting for White(X) Input...\n";
        int receivedDataLen = recv(hClntSock, (char *)&peerMove, sizeof(MoveData), 0);
        if (receivedDataLen <= 0)
        {
            std::cout << "White(X) Disconnected...\n";
            break;
        }
        int peerRow = ntohl(peerMove.row);
        int peerCol = ntohl(peerMove.col);
        gameBoard.placeStone(peerRow-1, peerCol-1, peerStone);
        
        if (gameBoard.checkWin(peerRow - 1, peerCol - 1, Stone::White))
        {
            system("cls");
            gameBoard.printBoard();
            std::cout << "*** White(X) Win!!! ***\n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            break;
        }
    }
    /*통신 테스트용 코드 종료*/
    closesocket(hClntSock);
    closesocket(hServSock);
    WSACleanup();
    return 0;
}

void ErrorHandling(const std::string& message, SOCKET hServSock, SOCKET hClntSock)
{
    std::cerr << message << '\n';
    if (hServSock != INVALID_SOCKET) closesocket(hServSock);
    if (hClntSock!= INVALID_SOCKET) closesocket(hClntSock);
    WSACleanup();
    exit(1);
}