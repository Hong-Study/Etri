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
                if(TIM->Start())
                    MessageBox(hwnd, _T("서버가 정상적으로 실행되었습니다."), _T("알림"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
                else
                {
                    MessageBox(hwnd, _T("비정상적인 에러가 발생하였습니다."), _T("알림"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
                    PostQuitMessage(0);
                }
            }
            return TRUE;
        case ID_PORT:
            DialogBoxParam(WINGUI->GetHInstance(), MAKEINTRESOURCE(PORT_OPTION), hwnd, PortOptionProc, 0);
            return TRUE;
        case ID_TEST:
            DialogBoxParam(WINGUI->GetHInstance(), MAKEINTRESOURCE(TEST_OPTION), hwnd, TestOptionProc, 0);
            return TRUE;
        case ID_LORA:
            DialogBoxParam(WINGUI->GetHInstance(), MAKEINTRESOURCE(LORA_INFO), hwnd, LoraInformation, 0);
            return TRUE;
        }
        return TRUE;

    case WM_NOTIFY:
        if (LOWORD(wParam) == IDC_TAB)
        {
            WINGUI->SelectTab();
        }
        else
        {
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
    static HWND checkDistance, checkCount;
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

        checkDistance = GetDlgItem(hwnd, IDC_CHECK_DISTANCE);
        SetWindowText(checkDistance, to_wstring(GTrainSeparationCheckDistance).c_str());

        checkCount = GetDlgItem(hwnd, IDC_CHECK_COUNT);
        SetWindowText(checkCount, to_wstring(GTrainSeparationCheckCount).c_str());

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
            textLength = GetWindowTextLength(checkDistance);
            if (textLength > 0)
            {
                GetWindowText(checkDistance, buf, sizeof(buf) / sizeof(TCHAR));
                GTrainSeparationCheckDistance = _tstoi(buf);
            }
            textLength = GetWindowTextLength(checkCount);
            if (textLength > 0)
            {
                GetWindowText(checkCount, buf, sizeof(buf) / sizeof(TCHAR));
                GTrainSeparationCheckCount = _tstoi(buf);
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
    static int32 infoId = 0;
    switch (msg)
    {
    case WM_INITDIALOG:
        infoId = static_cast<int>(lParam);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            WINGUI->ClearInfomation(infoId);
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        return TRUE;
    case WM_NOTIFY:
        NMHDR* pnmhdr = reinterpret_cast<NMHDR*>(lParam);
        if (pnmhdr->code == LVM_GETGROUPINFO)
        {
            NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(pnmhdr);
            //WINGUI->UpdateInfo
        }
        return TRUE;
    }
    return FALSE;
}

LRESULT LoraInformation(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND loraInfo;
    RECT rect{};
    int32 width;
    int32 height;
    StLoraInfo info;
    LVITEM item{};
    LVCOLUMN colum{};
    switch (msg)
    {
    case WM_INITDIALOG:
        GetWindowRect(hwnd, &rect);

        width = (rect.right - rect.left) - 100;
        height = rect.bottom - rect.top - 150;
        info = TIM->GetLoraInfo();

        loraInfo = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT
            , 20, 20, width, height
            , hwnd, nullptr, WINGUI->GetHInstance(), nullptr);
        if (loraInfo == NULL)
        {
            EndDialog(hwnd, 0);
            return false;
        }

        colum.mask = LVCF_TEXT | LVCF_WIDTH;
        colum.pszText = (LPWSTR)L"로라 채널";
        colum.cx = width/2;
        ListView_InsertColumn(loraInfo, 0, &colum);

        colum.mask = LVCF_TEXT | LVCF_WIDTH;
        colum.pszText = (LPWSTR)L"사용 유무";
        colum.cx = width/2;
        ListView_InsertColumn(loraInfo, 1, &colum);

        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.iSubItem = 0;
        item.pszText = (LPWSTR)L"LORA CH : 1";
        ListView_InsertItem(loraInfo, &item);

        item.iSubItem = 1;
        item.pszText = (LPWSTR)L"Default";
        ListView_SetItem(loraInfo, &item);

        for (int i = 1;i < LORA_MAX_CH - 1;i++)
        {
            item.iItem = i;
            item.iSubItem = 0;
            wstring str = std::format(L"LORA CH : {0}", i+1);
            item.pszText = (LPWSTR)(str.c_str());
            ListView_InsertItem(loraInfo, &item);

            item.iSubItem = 1;
            if (info.bUse[i+1])
                item.pszText = (LPWSTR)L"TRUE";
            else
                item.pszText = (LPWSTR)L"FALSE";
            ListView_SetItem(loraInfo, &item);
        }

        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            CloseWindow(loraInfo);
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}
