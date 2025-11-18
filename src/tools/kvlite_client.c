#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 6379

void print_usage(const char* program_name)
{
    printf("Usage:\n");
    printf("  %s [command] [arguments...]\n", program_name);
    printf("  %s -h <host> -p <port> [command] [arguments...]\n", program_name);
    printf("\nExamples:\n");
    printf("  %s SET mykey myvalue\n", program_name);
    printf("  %s GET mykey\n", program_name);
    printf("  %s -h 192.168.1.100 GET temperature\n", program_name);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian_Russia.1251");
    system("chcp 1251 > nul");
    if (argc < 2) 
    {
        print_usage(argv[0]);
        return 1;
    }

    char host[256];
    int port = DEFAULT_PORT;
    strcpy_s(host, sizeof(host), DEFAULT_HOST);

    int first_arg = 1;

    if (argc >= 4 && strcmp(argv[1], "-h") == 0 && strcmp(argv[3], "-p") == 0) 
    {
        strcpy_s(host, sizeof(host), argv[2]);
        port = atoi(argv[4]);
        first_arg = 5;
    }
    else if (argc >= 3 && strcmp(argv[1], "-h") == 0) 
    {
        strcpy_s(host, sizeof(host), argv[2]);
        first_arg = 3;
    }
    else if (argc >= 3 && strcmp(argv[1], "-p") == 0) 
    {
        port = atoi(argv[2]);
        first_arg = 3;
    }

    if (first_arg >= argc) 
    {
        print_usage(argv[0]);
        return 1;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
    {
        fprintf(stderr, "Error: Failed to initialize Winsock\n");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        fprintf(stderr, "Error: Failed to create socket\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) 
    {
        fprintf(stderr, "Error: Invalid IP address '%s'\n", host);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) 
    {
        fprintf(stderr, "Error: Failed to connect to %s: %d\n", host, port);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char request[2048] = { 0 };
    for (int i = first_arg; i < argc; ++i) 
    {
        if (i > first_arg) strcat_s(request, sizeof(request), " ");
        strcat_s(request, sizeof(request), argv[i]);
    }
    strcat_s(request, sizeof(request), "\n");

    if (send(sock, request, (int)strlen(request), 0) == SOCKET_ERROR) 
    {
        fprintf(stderr, "Error: Failed to send data\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char response[4096];
    int bytes = recv(sock, response, sizeof(response) - 1, 0);
    if (bytes > 0) 
    {
        response[bytes] = '\0';
        size_t len = strlen(response);
        if (len > 0 && response[len - 1] == '\n') {
            response[len - 1] = '\0';
        }
        printf("%s\n", response);
    }
    else if (bytes == 0) 
    {
        printf("(the connection is closed)\n");
    }
    else 
    {
        fprintf(stderr, "Error: Failed to get response\n");
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}