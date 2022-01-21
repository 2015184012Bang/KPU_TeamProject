#include "PCH.h"

bool SocketUtil::StaticInit()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        ReportError("Starting Up");
        return false;
    }

    return true;
}

void SocketUtil::CleanUp()
{
    WSACleanup();
}

void SocketUtil::ReportError(const char* InOperationDesc)
{
    LPVOID lpMsgBuf;
    DWORD errorNum = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorNum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    HB_LOG("Error &s: &d- %s", InOperationDesc, errorNum, lpMsgBuf);

}

int SocketUtil::GetLastError()
{
    return WSAGetLastError();
}

TCPSocketPtr SocketUtil::CreateTCPSocket()
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s != INVALID_SOCKET)
    {
        return TCPSocketPtr(new TCPSocket(s));
        
    }
    else
    {
        ReportError("SocketUtil::CreateTCPSocket");
        return nullptr;
    }
}
