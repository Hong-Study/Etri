#include "pch.h"
#include "DialogFunctions.h"
#include "Global.h"
#include "WinApi.h"
#include "TIMServer.h"

LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int32 input, selectedIndex;
    NMHDR* pnmhdr;

    switch (msg)
    {
    case WM_INITDIALOG:
        WINGUI->SetDlgHwnd(hwnd);
        WINGUI->Init();
        WINGUI->AddLogList(L"Program Start!!!!!");
        return TRUE;

    case WM_COMMAND:
        input = LOWORD(wParam);
        switch (input)
        {
        case IDCANCEL:
            PostQuitMessage(0);
            return FALSE;
        case IDC_START:
            if (!GStart)
            {
                GStart = true;
                WINGUI->Start();
                TIM->SendKeepAlive();
                MessageBox(hwnd, _T("서버가 정상적으로 실행되었습니다."), _T("알림"), MB_OK | MB_ICONINFORMATION);
            }
            return TRUE;
        case ID_PORT:
            DialogBoxParam(WINGUI->GetHInstance(), MAKEINTRESOURCE(PORT_OPTION), hwnd, PortOptionProc, 0);
            return TRUE;
        case ID_TEST:
            DialogBoxParam(WINGUI->GetHInstance(), MAKEINTRESOURCE(TEST_OPTION), hwnd, TestOptionProc, 0);
            return TRUE;
        }
        return TRUE;

    case WM_NOTIFY:
        if (LOWORD(wParam) == IDC_TAB)
        {
            WINGUI->SelectTab();
        }
        // 더블클릭시 Information 출현
        pnmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (pnmhdr->code == NM_DBLCLK)
        {
            HWND hListView = pnmhdr->hwndFrom;
            selectedIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (selectedIndex >= 0)
            {
                switch (pnmhdr->idFrom)
                {
                case TIFD_LIST:
                    WINGUI->ShowTifdInformation(selectedIndex);
                    break;
                case TIRD_LIST:
                    WINGUI->ShowTirdInformation(selectedIndex);
                    break;
                case PAIRING_LIST:
                    WINGUI->ShowPairingInformation(selectedIndex);
                    break;
                }
            }
        }
        return TRUE;
    case WM_CLOSE:
        PostQuitMessage(0);
        return FALSE;

    case WM_DESTROY:
        PostQuitMessage(0);
        return FALSE;
    }

    return FALSE;
}

LRESULT TestOptionProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND maxDistance = 0, lowestSpeed = 0, failedDistance = 0, successCount = 0;
    TCHAR buf[256] = { };
    int32 textLength = 0;
    UNREFERENCED_PARAMETER(lParam);
    switch (msg)
    {
    case WM_INITDIALOG:
        maxDistance = GetDlgItem(hwnd, IDC_DISTANCE);
        SetWindowText(maxDistance, to_wstring(GMaximumDistance).c_str());

        lowestSpeed = GetDlgItem(hwnd, IDC_SPEED);
        SetWindowText(lowestSpeed, to_wstring(GLowestSpeed).c_str());

        failedDistance = GetDlgItem(hwnd, IDC_FAILD);
        SetWindowText(failedDistance, to_wstring(GDistanceAccuarcy).c_str());

        successCount = GetDlgItem(hwnd, IDC_COMPLETE);
        SetWindowText(successCount, to_wstring(GSuccessCount).c_str());

        return true;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            textLength = GetWindowTextLength(maxDistance);
            if (textLength > 0)
            {
                GetWindowText(maxDistance, buf, sizeof(buf) / sizeof(TCHAR));
                GMaximumDistance = _tstoi(buf);
            }
            textLength = GetWindowTextLength(lowestSpeed);
            if (textLength > 0)
            {
                GetWindowText(lowestSpeed, buf, sizeof(buf) / sizeof(TCHAR));
                GLowestSpeed = _tstoi(buf);
            }
            textLength = GetWindowTextLength(failedDistance);
            if (textLength > 0)
            {
                GetWindowText(failedDistance, buf, sizeof(buf) / sizeof(TCHAR));
                GDistanceAccuarcy = _tstoi(buf);
            }
            textLength = GetWindowTextLength(successCount);
            if (textLength > 0)
            {
                GetWindowText(successCount, buf, sizeof(buf) / sizeof(TCHAR));
                GSuccessCount = _tstoi(buf);
            }
            EndDialog(hwnd, LOWORD(wParam));
        }
        return TRUE;
    case WM_SYSCOMMAND:
        if (wParam == SC_MAXIMIZE)
        {
            // Dialog 최대화
            ShowWindow(hwnd, SW_MAXIMIZE);
            return TRUE;
        }
        else if (wParam == SC_MINIMIZE)
        {
            // Dialog 최소화
            ShowWindow(hwnd, SW_MINIMIZE);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

LRESULT PortOptionProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HWND serverPort = 0;
    TCHAR buf[256] = {};
    switch (msg)
    {
    case WM_INITDIALOG:
        serverPort = GetDlgItem(hwnd, SERVER_PORT);
        SetWindowText(serverPort, to_wstring(GServerPort).c_str());
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            int32 textLength = GetWindowTextLength(serverPort);
            if (textLength > 0)
            {
                GetWindowText(serverPort, buf, sizeof(buf) / sizeof(TCHAR));
                GServerPort = _tstoi(buf);
            }
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}

LRESULT InformationProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static int32 threadId = 0;
    switch (msg)
    {
    case WM_INITDIALOG:
        threadId = static_cast<int>(lParam);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            WINGUI->ClearInfomation(threadId);
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
    }
    return FALSE;
}