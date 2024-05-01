
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
string nickname;
int colorNumber;
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
COORD point{ 0,0 };
DWORD WINAPI Sender(void* param)
{
    while (true) {
        // cout << "Please insert your query for server: ";
        char query[DEFAULT_BUFLEN - 20];
        cin.getline(query, DEFAULT_BUFLEN - 20);
        string fullMessage = nickname + " " + to_string(colorNumber) + " " + query;
        send(client_socket, fullMessage.c_str(), fullMessage.length(), 0);

        // альтернативный вариант ввода данных стрингом
        // string query;
        // getline(cin, query);
        // send(client_socket, query.c_str(), query.size(), 0);
    }
}

DWORD WINAPI Receiver(void* param)
{
    while (true) {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';
        string check = response;
        if (check.find(nickname) == string::npos || check.find(nickname) == 0) {
            istringstream iss(response);
            int colorNumb;
            string nick;
            string msg;

            // Извлекаем номер цвета, ник и сообщение
            iss >> nick >> colorNumb;
            getline(iss, msg);

            SetConsoleTextAttribute(h, colorNumb); // Устанавливаем цвет перед выводом
            cout << nick << ": " << msg << endl;
            SetConsoleTextAttribute(h, 7); // Возвращаем стандартный цвет
            point.Y++;
        }
    }
}

BOOL ExitHandler(DWORD whatHappening)
{
    switch (whatHappening)
    {
    case CTRL_C_EVENT: // closing console by ctrl + c
    case CTRL_BREAK_EVENT: // ctrl + break
    case CTRL_CLOSE_EVENT: // closing the console window by X button
      return(TRUE);
        break;
    default:
        return FALSE;
    }
}

int main()
{
    // обработчик закрытия окна консоли
    //SetConsoleCtrlHandler((PHANDLER_ROUTINE)ExitHandler, true);

    system("title Client");

    // initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // разрешить адрес сервера и порт
    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // создать сокет на стороне клиента для подключения к серверу
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (client_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }

        // коннект к серверу
        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }

    cout << "Enter your nickname: ";
    getline(cin, nickname);

    cout << "Enter the color number: ";
    cin >> colorNumber;
    cin.ignore();
    string message = "Joined to server!";

    string fullMessage = nickname + " " + to_string(colorNumber) + " " + message;

    send(client_socket, fullMessage.c_str(), fullMessage.length(), 0);
    point.Y++;
    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
}