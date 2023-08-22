#include "pch.h"
#include "AcceptServer.h"
#include "TIMServer.h"
#include "ThreadManager.h"
#include "WinApi.h"
#include "PythonMap.h"
#include "DialogFunctions.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        CRASH("ERROR");

    // 초기화
    MAP->Init();
    TIM->Init();
    THREAD->Init();
    SERVER->Init();
    
    // WINAPI Instance 저장
    WINGUI->SetInstacne(hInstance);

    // WinApi 함수 호출
	//DialogBoxParam();
    HWND hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), nullptr, DialogProc, 0);
    if (hwnd == NULL)
    {
        CRASH("Wranning");
    }

    MSG msg{};

    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
            WINGUI->Execute();
    }

    GStart = false;

    TIM->Clear();
    WINGUI->Clear();
    SERVER->Clear();
    MAP->Clear();

    // 모든 스레드가 종료될 때 까지 기다림
    THREAD->Join();
    
    return true;
}