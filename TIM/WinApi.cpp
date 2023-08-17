#include "pch.h"
#include "WinApi.h"
#include "resource.h"
#include "ThreadManager.h"
#include "TIMServer.h"
#include "AcceptServer.h"
#include "DialogFunctions.h"
#include "TifdSession.h"
#include "TirdSession.h"
#include "PythonMap.h"

using namespace std;

void WinApi::Init()
{
    // GDI+ 관련된 어떤 함수라도 사용 전에 해당 함수를 호출해야 합니다.
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    int32 emptySize = 100;
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN) - emptySize;
    logWidth = screenWidth / 5;
    logHeight = screenHeight / 1.5;
    startX = 10;
    startY = 10;

    mainTab = GetDlgItem(dlgHwnd, IDC_TAB);
    startButton = GetDlgItem(dlgHwnd, IDC_START);
    closeButton = GetDlgItem(dlgHwnd, IDCANCEL);

    SetTabSize();
    SetTabText();
    SetButtonPos();
    CreateText();
    CreatePendingListView();
    CreatePairingListView();
    CreateLogView();

    ShowWindow(pendingTifdList, SW_SHOW);
    ShowWindow(pendingTirdList, SW_SHOW);
    ShowWindow(pairingList, SW_HIDE);
    ShowWindow(dlgHwnd, SW_MAXIMIZE);

    UpdateTime();

    startTimeInfo = WstringToString(timeInfo);
}

void WinApi::Clear()
{
    WRITE_LOCK_IDX(LOG_LOCK);

    SaveLogData();

    if(IsInfo.load() != 0)
    {
        EndDialog(infoHandle->dialog, 0);
        mapInfo = nullptr;
        infoHandle = nullptr;
    }

    tirdHashMap.clear();
    tifdHashMap.clear();
    pairingHashMap.clear();
    pairingItems.clear();
}

void WinApi::Start()
{
    THREAD->Push([=]() {
        SERVER->Update();
        });
}

void WinApi::SelectTab()
{
    int selectedIndex = TabCtrl_GetCurSel(mainTab);

    switch (selectedIndex)
    {
    case 0:
        ShowWindow(pendingTifdList, SW_SHOW);
        ShowWindow(pendingTirdList, SW_SHOW);
        ShowWindow(tifdText, SW_SHOW);
        ShowWindow(tirdText, SW_SHOW);
        ShowWindow(pairingList, SW_HIDE);
        break;
    case 1:
        ShowWindow(pendingTifdList, SW_HIDE);
        ShowWindow(pendingTirdList, SW_HIDE);
        ShowWindow(tifdText, SW_HIDE);
        ShowWindow(tirdText, SW_HIDE);
        ShowWindow(pairingList, SW_SHOW);
        break;
    }
}

void WinApi::SetTabSize()
{
    tabWidth = screenWidth - logWidth - 50;
    tabHeight = screenHeight;
    SetWindowPos(mainTab, HWND_TOP, startX, startY, tabWidth, tabHeight, 0);
    startX += 20;
    startY += 30;
}

void WinApi::SetTabText()
{
    TCITEM tie{};
    tie.mask = TCIF_TEXT | TCIF_PARAM;
    tie.pszText = (LPWSTR)L"Register List";
    TabCtrl_InsertItem(mainTab, 0, &tie);

    tie.pszText = (LPWSTR)L"Pairing List";
    TabCtrl_InsertItem(mainTab, 1, &tie);
}

void WinApi::SetButtonPos()
{
    // emptySize = 100
    int32 width = 100;
    int32 height = 50;
    SetWindowPos(startButton, HWND_TOP, tabWidth + startX, logHeight + startY + 100, width, height, 0);
    SetWindowPos(closeButton, HWND_TOP, tabWidth + startX + width*2, logHeight + startY + 100, width, height, 0);
}

void WinApi::ShowTifdInformation(int32 selectedIndex)
{
    //WRITE_LOCK;

    for (auto pairs : tifdHashMap)
    {
        if (pairs.second->pos == selectedIndex)
        {
            int32 expected = 0;
            int32 desired = pairs.second->idNum << 4;
            if(IsInfo.compare_exchange_strong(expected, desired) == true)
            {
                HWND handle = CreateDialogParam(hInstance, MAKEINTRESOURCE(INFORMATION), dlgHwnd, InformationProc, static_cast<LPARAM>(desired));
                if (CreateTifdInformation(handle) == false)
                {
                    AddLogList(L"Can't Open Information");
                    CloseWindow(handle);
                }
            }
            break;
        }
    }
}

void WinApi::ShowTirdInformation(int32 selectedIndex)
{
    for (auto pairs : tirdHashMap)
    {
        if (pairs.second->pos == selectedIndex)
        {
            int32 expected = 0;
            int32 desired = pairs.second->idNum << 4;;
            if (IsInfo.compare_exchange_strong(expected, desired) == true)
            {
                HWND handle = CreateDialogParam(hInstance, MAKEINTRESOURCE(INFORMATION), dlgHwnd, InformationProc, static_cast<LPARAM>(desired));
                if (CreateTirdInformation(handle) == false)
                {
                    AddLogList(L"Can't Open Information");
                    CloseWindow(handle);
                }
            }
            break;
        }
    }
}

void WinApi::ShowPairingInformation(int32 selectedIndex)
{
    //WRITE_LOCK;
    
    if (selectedIndex % 2 == 1)
        selectedIndex -= 1;

    for (auto pairs : pairingHashMap)
    {
        if (pairs.second->pos == selectedIndex)
        {
            int32 expected = 0;
            int32 desired = pairs.second->tifd->idNum << 4;
            if (IsInfo.compare_exchange_strong(expected, desired) == true)
            {
                HWND handle = CreateDialogParam(hInstance, MAKEINTRESOURCE(INFORMATION), dlgHwnd, InformationProc, static_cast<LPARAM>(desired));
                CreatePairingInformation(handle);
            }
            break;
        }
    }
}

void WinApi::ShowTrainAlramStatus(int32 listId, std::wstring status)
{
    TCHAR str[126];
    _stprintf_s(str, 126, L"Train No. %d was %ws", listId, status.c_str());
    MessageBox(dlgHwnd, str, _T("알람 정보"), MB_OK | MB_ICONINFORMATION);
}

void WinApi::ResetTirdPendingList()
{
    ListView_DeleteAllItems(pendingTirdList);

    int32 i = 0;
    for (auto session : tirdHashMap)
    {
        session.second->pos = i++;
        AddPendingListInfo(session.second);
    }
}

void WinApi::ResetTifdPendingList()
{
    ListView_DeleteAllItems(pendingTifdList);

    int32 i = 0;
    for(auto session : tifdHashMap)
    {
        session.second->pos = i++;
        AddPendingListInfo(session.second);
    }
}

void WinApi::ResetPairingList()
{
    ListView_DeleteAllItems(pairingList);

    pairingSize = 0;
    for (PListPtr item : pairingItems)
    {
        item->pos = pairingSize;
        AddPairing(item);
        pairingSize += 2;
    }
}

void WinApi::CreatePendingListView()
{
    int32 width = tabWidth / 2 - startX;
    int32 height = tabHeight - startY;
    pendingTifdList = CreateWindow(WC_LISTVIEW, L"PendingTifdList"
        , WS_CHILD | WS_VISIBLE | LVS_REPORT
        , startX, startY, width, height
        , dlgHwnd, reinterpret_cast<HMENU>(TIFD_LIST)
        , hInstance, nullptr);

    // 컬럼 추가
    CreateTifdListColum(pendingTifdList, (width / (int)PendingTifdListViewCategory::CategorySize));
    
    pendingTirdList = CreateWindow(WC_LISTVIEW, L"PendingTirdList"
        , WS_CHILD | WS_VISIBLE | LVS_REPORT
        , startX + tabWidth/2, startY , width, height
        , dlgHwnd, reinterpret_cast<HMENU>(TIRD_LIST)
        , hInstance, nullptr);

    // 컬럼 추가
    CreateTirdListColum(pendingTirdList, (width / (int)PendingTirdListViewCategory::CategorySize));
}

void WinApi::CreatePairingListView()
{
    int32 width = tabWidth - 30;
    int32 height = tabHeight - 100;
    pairingList = CreateWindow(WC_LISTVIEW, L""
        , WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EX_GRIDLINES
        , startX, startY, width, height
        , dlgHwnd, reinterpret_cast<HMENU>(PAIRING_LIST)
        , hInstance, nullptr);

    ListView_SetExtendedListViewStyle(pairingList, LVS_EX_GRIDLINES);
    
    int32 columSize = width / (int)PairingListViewCategory::CategorySize;
    CreatePairingColum(pairingList, columSize);
}

void WinApi::CreateLogView()
{
    int32 textSize = 40;
    int32 startWidth = tabWidth + 20;
    int32 startHeight = startY - 20;
    logText = CreateWindow(L"static", L"LOGS", WS_CHILD | WS_VISIBLE, startWidth, startHeight, textSize, textSize, dlgHwnd, nullptr, hInstance, nullptr);

    logListView = CreateWindow(WC_LISTVIEW, L"Logs"
        , WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EX_GRIDLINES
        , startWidth, startHeight + 20, logWidth, logHeight
        , dlgHwnd, nullptr
        , hInstance, nullptr);

    ListView_SetExtendedListViewStyle(logListView, LVS_EX_GRIDLINES);

    CreateLogColum();

    ShowWindow(logText, SW_SHOW);
    ShowWindow(logListView, SW_SHOW);
}

void WinApi::CreateText()
{
    int32 textSize = 40;
    tifdText = CreateWindow(L"static", L"TIFD", WS_CHILD | WS_VISIBLE, startX, startY, textSize, textSize, dlgHwnd, nullptr, hInstance, nullptr);
    // TODO

    tirdText = CreateWindow(L"static", L"TIRD", WS_CHILD | WS_VISIBLE, startX + tabWidth/2, startY, textSize, textSize, dlgHwnd, nullptr, hInstance, nullptr);

    ShowWindow(tifdText, SW_SHOW);
    ShowWindow(tirdText, SW_SHOW);
    startY += textSize - 20;
}

bool WinApi::CreateTifdInformation(HWND parent)
{
    RECT rect{};
    if (GetWindowRect(parent, &rect) == false)
    {
        IsInfo.store(0);
        return false;
    }

    int32 width = (rect.right - rect.left) / 4;
    int32 height = rect.bottom - rect.top - 100;

    HWND tifdInfo = CreateWindow(WC_LISTVIEW, L"Logs", WS_CHILD | WS_VISIBLE | LVS_REPORT
        , 20, 20, width, height
        , parent, nullptr
        , hInstance, nullptr);

    if (tifdInfo == NULL)
    {
        IsInfo.store(0);
        return false;
    }

    CreateTifdInfoColum(tifdInfo, width / 2);

    // 후보자로 변경해줘야 된다.
    HWND CandidateInfo = CreateWindow(WC_LISTVIEW, L"Logs", WS_CHILD | WS_VISIBLE | LVS_REPORT
        , width + 30, 20, width, height
        , parent, nullptr
        , hInstance, nullptr);

    if (CandidateInfo == NULL)
    {
        IsInfo.store(0);
        return false;
    }

    CreateCandidateColum(CandidateInfo, width / 2);

    mapInfo = make_shared<MapPendingInfo>();
    infoHandle = make_shared<InfoHandle>(parent, tifdInfo, CandidateInfo);

    ShowWindow(parent, SW_SHOW);
    return true;
}

bool WinApi::CreatePairingInformation(HWND parent)
{

    RECT rect{};
    if (GetWindowRect(parent, &rect) == false)
    {
        IsInfo.store(0);
        return false;
    }

    int32 width = (rect.right - rect.left) / 4;
    int32 height = rect.bottom - rect.top - 100;

    HWND tifdInfo = CreateWindow(WC_LISTVIEW, L"Logs", WS_CHILD | WS_VISIBLE | LVS_REPORT
        , 20, 20, width, height
        , parent, nullptr
        , hInstance, nullptr);

    if (tifdInfo == NULL)
    {
        IsInfo.store(0);
        return false;
    }

    CreateTifdInfoColum(tifdInfo, width / 2);

    HWND tirdInfo = CreateWindow(WC_LISTVIEW, L"Logs", WS_CHILD | WS_VISIBLE | LVS_REPORT
        , width + 30, 20, width, height
        , parent, nullptr
        , hInstance, nullptr);

    if (tirdInfo == NULL)
    {
        IsInfo.store(0);
        return false;
    }

    CreateTirdInfoColum(tirdInfo, width / 2);

    infoHandle = make_shared<InfoHandle>(parent, tifdInfo, tirdInfo);
    mapInfo = make_shared<MapPairingInfo>();

    ShowWindow(parent, SW_SHOW);
    return true;
}

void WinApi::CreateTifdListColum(HWND handle, int32 columSize)
{
    LVCOLUMN colum{};

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Information";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::Information, &colum);
    
    colum.pszText = (LPWSTR)L"No";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_No, &colum);

    colum.pszText = (LPWSTR)L"Train No";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_TrainNo, &colum);

    colum.pszText = (LPWSTR)L"Device ID";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_DeviceID, &colum);

    colum.pszText = (LPWSTR)L"Latitude";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_Lat, &colum);

    colum.pszText = (LPWSTR)L"Longitude";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_Lon, &colum);

    colum.pszText = (LPWSTR)L"Speed(Km/h)";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_Speed, &colum);

    colum.pszText = (LPWSTR)L"Statellite";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTifdListViewCategory::TableTifd_Sat, &colum);
}

void WinApi::CreateCandidateColum(HWND handle, int32 columSize)
{
    LVCOLUMN colum{};

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Candidate";
    colum.cx = columSize;
    ListView_InsertColumn(handle, 0, &colum);

    colum.pszText = (LPWSTR)L"TIRD Info";
    colum.cx = columSize;
    ListView_InsertColumn(handle, 1, &colum);
}

void WinApi::CreateTirdListColum(HWND handle, int32 columSize)
{
    LVCOLUMN colum{};

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Information";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::Information, &colum);

    colum.pszText = (LPWSTR)L"No";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_No, &colum);

    colum.pszText = (LPWSTR)L"Device ID";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_DeviceID, &colum);

    colum.pszText = (LPWSTR)L"Latitude";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_Lat, &colum);

    colum.pszText = (LPWSTR)L"Longitude";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_Lon, &colum);

    colum.pszText = (LPWSTR)L"Speed(Km/h)";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_Speed, &colum);

    colum.pszText = (LPWSTR)L"Statellite";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_Sat, &colum);

    colum.pszText = (LPWSTR)L"Battery";
    colum.cx = columSize;
    ListView_InsertColumn(handle, PendingTirdListViewCategory::TableTird_Battery, &colum);
}

void WinApi::CreatePairingColum(HWND handle, int32 columSize)
{
    LVCOLUMN colum{};

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Information";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Information, &colum);

    colum.pszText = (LPWSTR)L"No";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_No, &colum);

    colum.pszText = (LPWSTR)L"Train No";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_TrainNo, &colum);

    colum.pszText = (LPWSTR)L"Type";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Type, &colum);

    colum.pszText = (LPWSTR)L"Device ID";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_DeviceId, &colum);

    colum.pszText = (LPWSTR)L"Latitude";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Latitude, &colum);

    colum.pszText = (LPWSTR)L"Longitude";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Longitude, &colum);

    colum.pszText = (LPWSTR)L"Speed(km/h)";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Speed, &colum);

    colum.pszText = (LPWSTR)L"Distacne(m)";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Distance, &colum);

    colum.pszText = (LPWSTR)L"Train Length(m)";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Length, &colum);

    colum.pszText = (LPWSTR)L"Satellite";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Sat, &colum);

    colum.pszText = (LPWSTR)L"Train Status";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Status, &colum);

    colum.pszText = (LPWSTR)L"Battery";
    colum.cx = columSize;
    ListView_InsertColumn(handle, (int)PairingListViewCategory::Table_Battery, &colum);
}

void WinApi::CreateTifdInfoColum(HWND handle, int32 columSize)
{
    LVCOLUMN colum{};
    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"TIFD Device";
    colum.cx = columSize;
    ListView_InsertColumn(handle, 0, &colum);

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Information";
    colum.cx = columSize;
    ListView_InsertColumn(handle, 1, &colum);

    InsertTifdInfo(handle);
}

void WinApi::CreateTirdInfoColum(HWND handle, int32 columSize)
{
    LVCOLUMN colum{};
    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"TIRD Device";
    colum.cx = columSize;
    ListView_InsertColumn(handle, 0, &colum);

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Information";
    colum.cx = columSize;
    ListView_InsertColumn(handle, 1, &colum);

    InsertTirdInfo(handle);
}


void WinApi::InsertTifdInfo(HWND handle)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 0;

    for (int i = 0;i < (int)TifdInfoCategory::SIZE;i++)
    {
        lvItem.pszText = (LPWSTR)(tifdInfoStr[i].c_str());
        lvItem.iItem = i;
        ListView_InsertItem(handle, &lvItem);
    }
}

void WinApi::InsertTirdInfo(HWND handle)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 0;

    for (int i = 0;i < (int)TirdInfoCategory::SIZE;i++)
    {
        lvItem.pszText = (LPWSTR)(tirdInfoStr[i].c_str());
        lvItem.iItem = i;
        ListView_InsertItem(handle, &lvItem);
    }
}

bool WinApi::CreateTirdInformation(HWND parent)
{
    RECT rect{};
    if (GetWindowRect(parent, &rect) == false)
    {
        IsInfo.store(0);
        return false;
    }

    int32 width = (rect.right - rect.left) / 4;
    int32 height = rect.bottom - rect.top - 100;

    HWND tirdInfo = CreateWindow(WC_LISTVIEW, L"Logs", WS_CHILD | WS_VISIBLE | LVS_REPORT
        , 20, 20, width, height
        , parent, nullptr
        , hInstance, nullptr);

    if (tirdInfo == NULL)
    {
        IsInfo.store(0);
        return false;
    }

    CreateTirdInfoColum(tirdInfo, width / 2);

    ShowWindow(parent, SW_SHOW);

    mapInfo = make_shared<MapPendingInfo>();
    infoHandle = make_shared<InfoHandle>(parent, nullptr, tirdInfo);

    return true;
}

void WinApi::ClearInfomation(int32 id)
{
    if (IsInfo.load() == 0)
        return;

    int32 desired = -1;
    while (true) {
        int32 expected = id;
        if (IsInfo.compare_exchange_weak(expected, desired))
        {
            if (mapInfo != nullptr)
                mapInfo = nullptr;

            if (infoHandle != nullptr)
            {
                EndDialog(infoHandle->dialog, 0);
                infoHandle->SetNullptr();
                infoHandle = nullptr;
            }

            IsInfo.store(0);
            break;
        }
    }
}

void WinApi::CreateLogColum()
{
    LVCOLUMN colum{};

    colum.mask = LVCF_TEXT | LVCF_WIDTH;
    colum.pszText = (LPWSTR)L"Time";
    colum.cx = logWidth / 2 - 50;
    ListView_InsertColumn(logListView, (int)LogListViewCategory::Time, &colum);

    colum.pszText = (LPWSTR)L"Contents";
    colum.cx = logWidth / 2 + 50;
    ListView_InsertColumn(logListView, (int)LogListViewCategory::Contents, &colum);
}

void WinApi::AddPendingListInfo(TirdListPtr item)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(info.c_str());
    lvItem.iItem = item->pos;
    lvItem.iSubItem = 0;
    ListView_InsertItem(pendingTirdList, &lvItem);

    SetPendingListInfo(item);
}

void WinApi::SetPendingListInfo(TirdListPtr item)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(item->id.c_str());
    lvItem.iItem = item->pos;
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_No;
    ListView_SetItem(pendingTirdList, &lvItem);

    lvItem.pszText = (LPWSTR)(item->device.c_str());
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_DeviceID;
    ListView_SetItem(pendingTirdList, &lvItem);

    lvItem.pszText = (LPWSTR)(item->latitude.c_str());
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_Lat;
    ListView_SetItem(pendingTirdList, &lvItem);

    lvItem.pszText = (LPWSTR)(item->longitude.c_str());
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_Lon;
    ListView_SetItem(pendingTirdList, &lvItem);

    lvItem.pszText = (LPWSTR)(item->speed.c_str());
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_Speed;
    ListView_SetItem(pendingTirdList, &lvItem);

    lvItem.pszText = (LPWSTR)(item->sat.c_str());
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_Sat;
    ListView_SetItem(pendingTirdList, &lvItem);

    lvItem.pszText = (LPWSTR)(item->battery.c_str());
    lvItem.iSubItem = (int)PendingTirdListViewCategory::TableTird_Battery;
    ListView_SetItem(pendingTirdList, &lvItem);
}

void WinApi::InsertPendingTirdList(TirdListPtr item)
{
    auto it = tirdHashMap.insert({ item->idNum, item });
    if (it.second == false)
        CRASH("Error");
}

void WinApi::SetInfoCandidateItem(const vector<PossiblePairingList>& lists)
{
    ListView_DeleteAllItems(infoHandle->tirdInfo);

    int i = 0;

    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(L"ID");
    lvItem.iSubItem = 0;
    lvItem.iItem = 0;
    ListView_InsertItem(infoHandle->tirdInfo, &lvItem);

    lvItem.pszText = (LPWSTR)(L"Distance");
    lvItem.iSubItem = 1;
    lvItem.iItem = 0;
    ListView_SetItem(infoHandle->tirdInfo, &lvItem);
    
    for (PossiblePairingList list : lists)
    {
        wstring id = list.target->GetDeviceIdToWString();
        lvItem.pszText = (LPWSTR)(id.c_str());
        lvItem.iSubItem = 0;
        lvItem.iItem = 1;
        ListView_InsertItem(infoHandle->tirdInfo, &lvItem);

        wstring distance = to_wstring(list.currentDistacne);
        lvItem.pszText = (LPWSTR)(distance.c_str());
        lvItem.iSubItem = 1;
        lvItem.iItem = 1;
        ListView_SetItem(infoHandle->tirdInfo, &lvItem);
    }
}

void WinApi::SetInfoMapCreate(const pair<float, float> nowLocation)
{
    if (infoHandle == nullptr || mapInfo == nullptr)
        return;
    shared_ptr<MapPendingInfo> mapPendingInfo = static_pointer_cast<MapPendingInfo>(mapInfo);
    if (mapPendingInfo == nullptr)
        return;
    mapPendingInfo->SetPos(nowLocation);
    if (MAP->CreatePendingMapPng(mapPendingInfo->centerLat, mapPendingInfo->centerLong
        , mapPendingInfo->posLat, mapPendingInfo->posLong) == false)
        CRASH("");

    DrawPngMap(infoHandle->dialog);
}

void WinApi::SetInfoMapCreate(float lat1, float long1, float lat2, float long2)
{
    if (infoHandle == nullptr || mapInfo == nullptr)
        return;
    shared_ptr<MapPairingInfo> mapPairingInfo = static_pointer_cast<MapPairingInfo>(mapInfo);
    if (mapPairingInfo == nullptr)
        return;
    mapPairingInfo->SetPos(lat1, long1, lat2, long2);
    if (MAP->CreatePairingMapPng(
        mapPairingInfo->centerLat, mapPairingInfo->centerLong
        , mapPairingInfo->tifdLat, mapPairingInfo->tifdLong
        , mapPairingInfo->tirdLat, mapPairingInfo->tirdLong) == false)
        CRASH("");

    DrawPngMap(infoHandle->dialog);
}

void WinApi::DrawPngMap(const HWND handle)
{
    // image.png 파일을 이용하여 Image 객체를 생성합니다.
    HDC hdc = GetDC(handle);
    Gdiplus::Image* image = Gdiplus::Image::FromFile(L"SaveMap/map.png");
    Gdiplus::Graphics g(hdc);

    // (x, y)에 width X height 크기의 이미지를 그립니다.
    g.DrawImage(image, 700, 25, 400, 400);

    // 데이터 메모리 해제
    delete image;
    ReleaseDC(handle, hdc);
}

void WinApi::SetInfoTifdItem(const HWND handle, const TifdListPtr tifd, const std::wstring pairState, const std::wstring tirdDeviceId)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(tifd->version.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Version;
    lvItem.iSubItem = 1;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->device.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Device;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(pairState.c_str());
    lvItem.iItem = (int)TifdInfoCategory::PairingStatus;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->ip.c_str());
    lvItem.iItem = (int)TifdInfoCategory::IpAddress;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->port.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Port;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->time.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Time;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->latitude.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Latitude;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->longitude.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Longitude;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->altitude.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Altitude;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->speed.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Speed;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->sat.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Satellite;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->distance.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Distance;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->loraVersion.c_str());
    lvItem.iItem = (int)TifdInfoCategory::LoRaVersion;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->loraCh.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Channel;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->loraPower.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Power;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->loraBW.c_str());
    lvItem.iItem = (int)TifdInfoCategory::Bandwidth;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(GSpreadingFactor.c_str());
    lvItem.iItem = (int)TifdInfoCategory::SpreadingFactor;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(GCodingRate.c_str());
    lvItem.iItem = (int)TifdInfoCategory::CodingRate;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tirdDeviceId.c_str());
    lvItem.iItem = (int)TifdInfoCategory::TIRD_Device;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->rssi.c_str());
    lvItem.iItem = (int)TifdInfoCategory::RSSI;
    ListView_SetItem(handle, &lvItem);
}

void WinApi::SetInfoTirdItem(const HWND handle, const TirdListPtr tird, std::wstring pairState)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(tird->version.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Version;
    lvItem.iSubItem = 1;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->device.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Device;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(pairState.c_str());
    lvItem.iItem = (int)TirdInfoCategory::PairingStatus;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->ip.c_str());
    lvItem.iItem = (int)TirdInfoCategory::IpAddress;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->port.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Port;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->time.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Time;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->latitude.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Latitude;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->longitude.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Longitude;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->altitude.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Altitude;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->speed.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Speed;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->sat.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Satellite;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->loraCh.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Channel;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->loraPower.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Power;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->loraBW.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Bandwidth;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(GSpreadingFactor.c_str());
    lvItem.iItem = (int)TirdInfoCategory::SpreadingFactor;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(GCodingRate.c_str());
    lvItem.iItem = (int)TirdInfoCategory::CodingRate;
    ListView_SetItem(handle, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->battery.c_str());
    lvItem.iItem = (int)TirdInfoCategory::Battery_Voltage;
    ListView_SetItem(handle, &lvItem);
}

void WinApi::AddLogList(wstring contexts)
{
    WRITE_LOCK_IDX(LOG_LOCK);

    // 현재 시간을 시스템 클록으로부터 구합니다.
    UpdateTime();

    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(timeInfo.c_str());
    lvItem.iItem = logSize++;
    lvItem.iSubItem = (int)LogListViewCategory::Time;
    ListView_InsertItem(logListView, &lvItem);

    lvItem.pszText = (LPWSTR)(contexts.c_str());
    lvItem.iSubItem = (int)LogListViewCategory::Contents;
    ListView_SetItem(logListView, &lvItem);

    logList.push_back(pair{ timeInfo, contexts });
}

void WinApi::UpdateTime()
{
    std::tm localTime = GetLocalTime();

    timeInfo.clear();
    timeInfo.resize(20);
    swprintf_s(timeInfo.data(), 20, L"%d/%02d/%02d-%02d:%02d:%02d",
        localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
        localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
}

void WinApi::SaveLogData()
{
    static bool check = true;
    if (check)
    {
        startTimeInfo.insert(startTimeInfo.size() - 1, ".txt");
        startTimeInfo.replace(startTimeInfo.find("-"), 1, "/");

        while (int32 it = (int32)startTimeInfo.find(":"))
        {
            if (it == string::npos)
                break;
            startTimeInfo.replace(it, 1, "-");
        }

        string createPath = GLogPos + "GUI_Log/" + startTimeInfo;
        std::filesystem::path path(createPath);

        if (std::filesystem::exists(path.parent_path()) == false)
        {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream out(createPath);

        for (auto logs : logList)
        {
            wstring str = std::format(L"{0} -> {1}\n", logs.first, logs.second);
            string write = WstringToString(str);
            out << write;
        }

        out.close();
        check = false;
    }
}

int32 WinApi::NewPendingList(TirdRef session)
{
    WRITE_LOCK_IDX(TIFD_LOCK);

    StTirdData* data = session->GetData();
    if (data != nullptr)
    {
        wchar_t ipBuffer[INET6_ADDRSTRLEN];
        const SOCKADDR_IN& addr = session->GetSockAddr();
        InetNtop(AF_INET, &(addr.sin_addr), ipBuffer, INET6_ADDRSTRLEN);
        return NewPendingTirdList(data, ipBuffer, to_wstring(ntohs(addr.sin_port)));
    }
    else
        return -1;
}

int32 WinApi::NewPendingTirdList(StTirdData* data, wstring ip, wstring port)
{
    int32 id = sessionCount.fetch_add(1);
    int32 pos = static_cast<int32>(tirdHashMap.size());
    TirdListPtr item = make_shared<PendingTirdListViewItem>(id, pos, data);
    item->ip = ip;
    item->port = port;

    InsertPendingTirdList(item);
    AddPendingListInfo(item);
    
    return id;
}

void WinApi::InsertPendingTifdList(TifdListPtr item)
{
    auto it = tifdHashMap.insert({ item->idNum, item });
    if (it.second == false)
        CRASH("Error");
}

void WinApi::AddPendingListInfo(TifdListPtr item)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(info.c_str());
    lvItem.iItem = item->pos;
    lvItem.iSubItem = 0;
    ListView_InsertItem(pendingTifdList, &lvItem);

    SetPendingListInfo(item);
}

void WinApi::SetPendingListInfo(TifdListPtr item)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = item->pos;

    lvItem.pszText = (LPWSTR)(item->id.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_No;
    ListView_SetItem(pendingTifdList, &lvItem);
   
    lvItem.pszText = (LPWSTR)(item->train.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_TrainNo;
    ListView_SetItem(pendingTifdList, &lvItem);
    
    lvItem.pszText = (LPWSTR)(item->device.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_DeviceID;
    ListView_SetItem(pendingTifdList, &lvItem);
    
    lvItem.pszText = (LPWSTR)(item->latitude.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_Lat;
    ListView_SetItem(pendingTifdList, &lvItem);
    
    lvItem.pszText = (LPWSTR)(item->longitude.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_Lon;
    ListView_SetItem(pendingTifdList, &lvItem);
    
    lvItem.pszText = (LPWSTR)(item->speed.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_Speed;
    ListView_SetItem(pendingTifdList, &lvItem);
    
    lvItem.pszText = (LPWSTR)(item->sat.c_str());
    lvItem.iSubItem = (int)PendingTifdListViewCategory::TableTifd_Sat;
    ListView_SetItem(pendingTifdList, &lvItem);
}

int32 WinApi::NewPendingList(TifdRef session)
{
    WRITE_LOCK_IDX(TIFD_LOCK);

    StTifdData* data = session->GetData();
    if (data != nullptr)
    {
        wchar_t ipBuffer[INET6_ADDRSTRLEN];
        const SOCKADDR_IN& addr = session->GetSockAddr();
        InetNtop(AF_INET, &(addr.sin_addr), ipBuffer, INET6_ADDRSTRLEN);
        return NewPendingTifdList(data, ipBuffer, to_wstring(ntohs(addr.sin_port)));
    }

    return -1;
}

int32 WinApi::NewPendingTifdList(StTifdData* data, wstring ip, wstring port)
{
    int32 id = sessionCount.fetch_add(1);
    int32 pos = static_cast<int32>(tifdHashMap.size());
    TifdListPtr item = make_shared<PendingTifdListViewItem>(id, pos, data);
    item->ip = ip;
    item->port = port;

    InsertPendingTifdList(item);
    AddPendingListInfo(item);

    return id;
}

void WinApi::UpdateTifdPendingInfo(int32 id, TifdRef tifd)
{
    TifdListPtr ptr = nullptr;
    {
        WRITE_LOCK_IDX(TIFD_LOCK);

        auto it = tifdHashMap.find(id);
        if (it == tifdHashMap.end())
            return;

        ptr = it->second;
        ptr->SetInfo(tifd->GetData());
        SetPendingListInfo(ptr);
    }

    if (ptr == nullptr)
        return;

    int32 isInfo = id << 4;
    if (IsInfo.compare_exchange_weak(isInfo, isInfo + 1))
    {
        SetInfoTifdItem(infoHandle->tifdInfo, ptr, L"UnPair", tifd->GetDeviceIdToWString());
        SetInfoCandidateItem(tifd->_possibleLists);
        SetInfoMapCreate(tifd->GetLocation());
        IsInfo.store(isInfo);
    }
}

void WinApi::UpdateTirdPendingInfo(int32 id, StTirdData* data)
{
    TirdListPtr ptr = nullptr;
    {
        WRITE_LOCK_IDX(TIRD_LOCK);

        auto it = tirdHashMap.find(id);
        if (it == tirdHashMap.end())
            return;

        ptr = it->second;
        ptr->SetInfo(data);
        SetPendingListInfo(ptr);
    }
    
    if (ptr == nullptr)
        return;

    int32 isInfo = id << 4;
    if (IsInfo.compare_exchange_weak(isInfo, isInfo + 1))
    {
        SetInfoTirdItem(infoHandle->tirdInfo, ptr, L"Not Paired");
        SetInfoMapCreate(pair{ data->lat, data->lon });
        IsInfo.store(isInfo);
    }
}

void WinApi::UpdateTifdPairingInfo(int32 id, int32 distance, StTifdData* tifd, StTirdData* tird)
{
    PListPtr ptr = nullptr;
    {
        WRITE_LOCK_IDX(PAIRING_LOCK);

        auto it = pairingHashMap.find(id);
        if (it == pairingHashMap.end())
            return;

        ptr = it->second;
        ptr->tifd->SetInfo(tifd);
        ptr->distance = to_wstring(distance);

        SetTifdPairingList(ptr);
    }
    
    if (ptr == nullptr)
        return;

    int32 isId = ptr->tifd->idNum << 4;
    if (IsInfo.compare_exchange_weak(isId, isId + 1))
    {
        SetInfoTifdItem(infoHandle->tifdInfo, ptr->tifd, L"Paired", ptr->tird->device);
        SetInfoMapCreate(tifd->lat, tifd->lon, tird->lat, tird->lon);
        IsInfo.store(isId);
    }
}

void WinApi::UpdateTirdPairingInfo(int32 id, StTirdData* data)
{
    PListPtr ptr = nullptr;
    {
        WRITE_LOCK_IDX(PAIRING_LOCK);

        auto it = pairingHashMap.find(id);
        if (it == pairingHashMap.end())
            return;

        ptr = it->second;
        ptr->tird->SetInfo(data);

        SetTirdPairingList(ptr);
    }

    if (ptr == nullptr)
        return;
    
    int32 isId = ptr->tifd->idNum << 4;
    if (IsInfo.compare_exchange_weak(isId, isId + 1))
    {
        SetInfoTirdItem(infoHandle->tirdInfo, ptr->tird, L"Paired");
        IsInfo.store(isId);
    }
}

int32 WinApi::NewPairingList(int32 tifdId, int32 tirdId, int32 distance)
{
    WRITE_LOCK_IDX(PAIRING_LOCK);

    int32 id = pairingCount.fetch_add(1);
    int32 pos = pairingSize;
    TifdListPtr tifd;
    TirdListPtr tird;

    {
        WRITE_LOCK_IDX(TIFD_LOCK);
        auto it = tifdHashMap.find(tifdId);
        if (it == tifdHashMap.end())
            CRASH("Not Inside");
        tifd = it->second;
    }
    
    {
        WRITE_LOCK_IDX(TIRD_LOCK);
        auto it = tirdHashMap.find(tirdId);
        if (it == tirdHashMap.end())
            CRASH("Not Inside");
        tird = it->second;
    }
    
    PListPtr ptr = make_shared<PairingListViewItem>(id, pos, to_wstring(distance), tifd, tird);
    pairingItems.push_back(ptr);

    auto it = pairingHashMap.insert({ id, ptr });
    if (it.second == false)
        CRASH("AddPairing");

    AddPairing(ptr);
    pairingSize += 2;

    return id;
}

void WinApi::AddPairing(const PListPtr ptr)
{
    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(info.c_str());
    lvItem.iItem = ptr->pos;
    lvItem.iSubItem = 0;
    ListView_InsertItem(pairingList, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)(info.c_str());
    lvItem.iItem = ptr->pos + 1;
    lvItem.iSubItem = 0;
    ListView_InsertItem(pairingList, &lvItem);

    SetTifdPairingList(ptr);
    SetTirdPairingList(ptr);
}

void WinApi::SetTifdPairingList(const PListPtr ptr)
{
    const TifdListPtr tifd = ptr->tifd;

    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = ptr->pos;

    lvItem.pszText = (LPWSTR)(ptr->id.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_No;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->train.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_TrainNo;
    ListView_SetItem(pairingList, &lvItem);

    {
        lvItem.iItem = ptr->pos + 1;
        ListView_SetItem(pairingList, &lvItem);
        lvItem.iItem = ptr->pos;
    }

    lvItem.pszText = (LPWSTR)(L"TIFD");
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Type;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->device.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_DeviceId;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->latitude.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Latitude;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->longitude.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Longitude;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->speed.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Speed;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tifd->sat.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Sat;
    ListView_SetItem(pairingList, &lvItem);

    // 후에 바꿔야함.
    lvItem.pszText = (LPWSTR)(ptr->distance.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Distance;
    ListView_SetItem(pairingList, &lvItem);

    {
        lvItem.iItem = ptr->pos + 1;
        ListView_SetItem(pairingList, &lvItem);
        lvItem.iItem = ptr->pos;
    }

    lvItem.pszText = (LPWSTR)(tifd->trainLength.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Length;
    ListView_SetItem(pairingList, &lvItem);

    {
        lvItem.iItem = ptr->pos + 1;
        ListView_SetItem(pairingList, &lvItem);
        lvItem.iItem = ptr->pos;
    }

    lvItem.pszText = (LPWSTR)(tifd->trainStatus.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Status;
    ListView_SetItem(pairingList, &lvItem);

    {
        lvItem.iItem = ptr->pos + 1;
        ListView_SetItem(pairingList, &lvItem);
        lvItem.iItem = ptr->pos;
    }
}

void WinApi::SetTirdPairingList(const PListPtr ptr)
{
    const TirdListPtr tird = ptr->tird;

    LVITEM lvItem{};
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = ptr->pos + 1;

    lvItem.pszText = (LPWSTR)(ptr->id.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_No;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(L"TIRD");
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Type;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->device.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_DeviceId;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->latitude.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Latitude;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->longitude.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Longitude;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->speed.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Speed;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->sat.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Sat;
    ListView_SetItem(pairingList, &lvItem);

    lvItem.pszText = (LPWSTR)(tird->battery.c_str());
    lvItem.iSubItem = (int)PairingListViewCategory::Table_Battery;
    ListView_SetItem(pairingList, &lvItem);
}

void WinApi::DeleteTirdPendingList(int32 id)
{
    WRITE_LOCK_IDX(TIRD_LOCK);

    auto tirdIt = tirdHashMap.find(id);
    if (tirdIt == tirdHashMap.end())
    {
        return;
    }    
    else
    {
        int32 expected = IsInfo.load() >> 4;
        if (expected == id)
            ClearInfomation(expected << 4);
        tirdHashMap.erase(tirdIt);
        ResetTirdPendingList();
    }
}

void WinApi::DeleteTifdPendingList(int32 id)
{
    WRITE_LOCK_IDX(TIFD_LOCK);

    auto tifdIt = tifdHashMap.find(id);
    if (tifdIt == tifdHashMap.end())
    {
        return;
    }        
    else
    {
        int32 expected = IsInfo.load() >> 4;
        if (expected == id)
            ClearInfomation(expected << 4);
        tifdHashMap.erase(tifdIt);
        ResetTifdPendingList();
    }    
}

void WinApi::DeletePairingList(int32 id, Device device)
{
    WRITE_LOCK_IDX(PAIRING_LOCK);
    
    auto ptr = pairingHashMap.find(id);
    if (ptr == pairingHashMap.end())
        return;

    PListPtr pList = ptr->second;
    // 조금 위험도 높은 코드
    int32 pos = pList->pos / 2;
    pairingItems.erase(pairingItems.begin() + pos);
    pairingHashMap.erase(ptr);

    int32 expected = IsInfo.load() >> 4;
    if (expected == pList->tifd->idNum)
        ClearInfomation(expected << 4);
    ResetPairingList();

    if (device == Device::DeviceTIFD)
    {
        WRITE_LOCK_IDX(TIRD_LOCK);
        InsertPendingTirdList(pList->tird);
        ResetTirdPendingList();
    }
    else if (device == Device::DeviceTIRD)
    {
        WRITE_LOCK_IDX(TIFD_LOCK);
        InsertPendingTifdList(pList->tifd);
        ResetTifdPendingList();
    }
}

void WinApi::PairingDisconnectDection(uint32 trainNum)
{
    TCHAR str[126];
    _stprintf_s(str,126, L"Train No. %d was Disconnet Detected", trainNum);
    MessageBox(dlgHwnd, str, _T("끊김 알림"), MB_OK | MB_ICONINFORMATION);
}