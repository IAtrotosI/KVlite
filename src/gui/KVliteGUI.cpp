#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#undef _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <commctrl.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <string>
#include <sstream>

HWND hInput, hOutput, hSendButton, hHostEdit, hPortEdit;
char gHost[256] = "127.0.0.1";
int gPort = 6379;
SOCKET gSocket = INVALID_SOCKET;

bool connectToServer() {
    if (gSocket != INVALID_SOCKET) 
    {
        closesocket(gSocket);
        gSocket = INVALID_SOCKET;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    gSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (gSocket == INVALID_SOCKET) 
    {
        WSACleanup();
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)gPort);
    inet_pton(AF_INET, gHost, &addr.sin_addr);

    if (connect(gSocket, (sockaddr*)&addr, sizeof(addr)) != 0) 
    {
        closesocket(gSocket);
        gSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    return true;
}

std::string sendCommand(const std::string& cmd) {
    if (gSocket == INVALID_SOCKET && !connectToServer()) 
    {
        return "Error: Failed to connect";
    }

    std::string request = cmd + "\n";
    if (send(gSocket, request.c_str(), (int)request.size(), 0) == SOCKET_ERROR) 
    {
        closesocket(gSocket);
        gSocket = INVALID_SOCKET;
        WSACleanup();
        return "Error: sending data";
    }

    char buf[4096];
    int n = recv(gSocket, buf, sizeof(buf) - 1, 0);
    std::string resp = "No answer";
    if (n > 0) {
        buf[n] = '\0';
        resp = buf;
        if (!resp.empty() && resp.back() == '\n') 
        {
            resp.pop_back();
        }
    }
    else 
    {
        closesocket(gSocket);
        gSocket = INVALID_SOCKET;
        WSACleanup();
    }

    return resp;
}

void onSendCommand() 
{
    char inputText[1024];
    GetWindowTextA(hInput, inputText, sizeof(inputText));

    if (strlen(inputText) == 0) return;

    char hostText[256], portText[16];
    GetWindowTextA(hHostEdit, hostText, sizeof(hostText));
    GetWindowTextA(hPortEdit, portText, sizeof(portText));
    if (strlen(hostText) > 0) strcpy_s(gHost, hostText);
    if (strlen(portText) > 0) gPort = atoi(portText);

    std::string response = sendCommand(inputText);

    std::string toAppend = "> ";
    toAppend += inputText;
    toAppend += "\r\n";
    toAppend += response;
    toAppend += "\r\n";

    int len = GetWindowTextLengthA(hOutput);
    char* current = (char*)malloc(len + 1);
    GetWindowTextA(hOutput, current, len + 1);

    std::string fullText = current;
    if (!fullText.empty() && fullText.back() != '\n')
    {
        fullText += "\r\n";
    }
    fullText += toAppend;

    free(current);

    SetWindowTextA(hOutput, fullText.c_str());

    SendMessageA(hOutput, EM_SETSEL, -1, -1);
    SendMessageA(hOutput, EM_SCROLLCARET, 0, 0);

    SetWindowTextA(hInput, "");
    SetFocus(hInput);
}

LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) 
{
    if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) 
    {
        onSendCommand();
        return 0;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) 
    {
    case WM_CREATE:
        SetWindowSubclass(hInput, EditSubclassProc, 0, 0);
        break;

    case WM_COMMAND:
        if ((HWND)lParam == hSendButton) 
        {
            onSendCommand();
            return 0;
        }
        break;

    case WM_DESTROY:
        if (gSocket != INVALID_SOCKET) 
        {
            closesocket(gSocket);
            WSACleanup();
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) 
{
    const char CLASS_NAME[] = "KVliteGUIClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KVlite GUI Client",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL
    );
    if (!hwnd) return 1;

    CreateWindowA("STATIC", "host:", WS_CHILD | WS_VISIBLE, 10, 10, 50, 20, hwnd, NULL, hInstance, NULL);
    hHostEdit = CreateWindowA("EDIT", "127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        60, 8, 120, 24, hwnd, NULL, hInstance, NULL);

    CreateWindowA("STATIC", "Port:", WS_CHILD | WS_VISIBLE, 190, 10, 50, 20, hwnd, NULL, hInstance, NULL);
    hPortEdit = CreateWindowA("EDIT", "6379", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        240, 8, 60, 24, hwnd, NULL, hInstance, NULL);

    hInput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        10, 40, 450, 24, hwnd, NULL, hInstance, NULL);
    hSendButton = CreateWindowA("BUTTON", "Send", WS_CHILD | WS_VISIBLE,
        470, 40, 110, 24, hwnd, NULL, hInstance, NULL);

    hOutput = CreateWindowA("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        10, 70, 570, 300, hwnd, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    SetFocus(hInput);

    if (!connectToServer()) 
    {
        MessageBoxA(hwnd, "Could not connect to the server", "Error", MB_ICONERROR);
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (msg.message == WM_KEYDOWN && msg.hwnd == hInput && msg.wParam == VK_RETURN) 
        {
            onSendCommand();
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}