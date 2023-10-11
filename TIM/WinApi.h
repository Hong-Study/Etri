#pragma once
#include "JobQueue.h"

class WinApi : public JobQueue
{
public:
	void		Init();
	void		Clear();
	void		SetDlgHwnd(HWND hwnd) { dlgHwnd = hwnd; }
	HWND		GetDlgHwnd() { return dlgHwnd; }

	void		SetInstacne(HINSTANCE instance) { hInstance = instance; }
	HINSTANCE	GetHInstance() { return hInstance; }
	void		SelectTab();
	void		SaveLogData();

public:
	// 저장 되어 있는 정보들 업데이트
	void		UpdateTifdPendingInfo(int32 id, StTifdData data, vector<PossiblePairingList> possibleList);
	void		UpdateTirdPendingInfo(int32 id, StTirdData data);

	void		UpdateTifdPairingInfo(int32 id, int32 distance, StTifdData tifd, StTirdData tird);
	void		UpdateTirdPairingInfo(int32 id, StTirdData data);

	void		ChangePairingToTird(int32 pairingId, int32 tirdId);

	// Information Update
	void		TifdInfoUpdate(int32 infoId, pair<float, float> location
		, wstring tirdDeviceId, vector<PossiblePairingList>& possibleLists, TifdListPtr tifdList);
	void		TirdInfoUpdate(int32 infoId, pair<float, float> location, TirdListPtr tirdList);
	void		PairingInfoUpdate(int32 infoId, pair<float, float> tifdLocation, pair<float, float> tirdLocation
		, TifdListPtr tifdList, TirdListPtr tirdList);

	// Session 종료시 삭제
	void		DeleteTirdPendingList(int32 id);
	void		DeleteTifdPendingList(int32 id);
	void		DeletePairingList(int32 id, Device device);

	void		PairingDisconnectDection(uint32 id);
	void		AddLogList(wstring contexts);

	// Information 보기
	void		ShowTifdInformation(int32 selectedIndex);
	void		ShowTirdInformation(int32 selectedIndex);
	void		ShowPairingInformation(int32 selectedIndex);
	void		ShowTrainAlramStatus(int32 listId, std::wstring status);

	// Information 생성(ListView)
	bool		CreateTifdInformation(HWND parent);
	bool		CreateTirdInformation(HWND parent);
	bool		CreatePairingInformation(HWND parent);
	void		ClearInfomation(int32 id);

	// 새로운 PendingList 추가
	void		NewTifdPendingList(int32 id, TifdRef session);
	void		NewTirdPendingList(int32 id, TirdRef session);
	void		NewPairingList(int32 pairingId, int32 tifdListId, int32 tirdListId, int32 distance);

private:
	// 초기화 부분들
	void		SetTabSize();
	void		SetTabText();
	void		SetButtonPos();

	void		CreateText();
	void		CreatePendingListView();
	void		CreateTifdListColum(HWND handle, int32 columSize);
	void		CreateTirdListColum(HWND handle, int32 columSize);

	void		CreatePairingListView();
	void		CreatePairingColum(HWND handle, int32 columSize);

	// Information 생성 보조
	void		CreateCandidateColum(HWND handle, int32 columSize);
	void		CreateTifdInfoColum(HWND handle, int32 columSize);
	void		CreateTirdInfoColum(HWND handle, int32 columSize);
	// 공통
	void		InsertTifdInfo(HWND handle);
	void		InsertTirdInfo(HWND handle);

	void		CreateLogView();
	void		CreateLogColum();

private:
	// 다시	초기화하고 출력
	// 동시에 많이 하게 되면, 깜빡이는 현상 발생
	void		ResetTirdPendingList();
	void		ResetTifdPendingList();
	void		ResetPairingList();

	// Pending List 보조 함수
	void		NewPendingTifdList(int32 id, StTifdData data, wstring ip, wstring port);
	void		NewPendingTirdList(int32 id, StTirdData data, wstring ip, wstring port);

	// Pendinglist에 집어넣기
	void		InsertPendingTifdList(TifdListPtr item);
	void		InsertPendingTirdList(TirdListPtr item);

	void		AddPendingListInfo(TifdListPtr item);
	void		AddPendingListInfo(TirdListPtr item);

	void		SetPendingListInfo(TifdListPtr item);
	void		SetPendingListInfo(TirdListPtr item);

	// Pairing 보조 함수
	void		AddPairing(const PListPtr ptr);
	void		SetTifdPairingList(const PListPtr ptr);
	void		SetTirdPairingList(const PListPtr ppr);

	// Information Setting
	void		SetInfoCandidateItem(const vector<PossiblePairingList>& lists);

	void		SetInfoMapCreate(const pair<float, float> nowLocatoin);
	void		SetInfoMapCreate(const pair<float, float> tifdLoc, const pair<float, float> tirdLoc);
	void		DrawPngMap(const HWND handle);

	void		SetInfoTifdItem(const HWND handle, const TifdListPtr tifd, const std::wstring pairState, const std::wstring tirdDeviceId);
	void		SetInfoTirdItem(const HWND handle, const TirdListPtr tird, std::wstring pairState);

	// 시간	구하는 함수
	void 		UpdateTime();

	friend		LRESULT InformationProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	// 초기화 타이밍이 매우 매우 매우 아쉽다.
	HWND dlgHwnd = 0;
	HINSTANCE hInstance = 0;

private:
	HWND mainTab = 0, pendingTifdList = 0, pendingTirdList = 0, pairingList = 0;
	HWND startButton = 0, closeButton = 0;
	HWND tifdText = 0, tirdText = 0;
	HWND logListView = 0, logText = 0;

	vector<pair<wstring, wstring>> logList;

private:
	// 리스트 저장
	vector<PListPtr>		pairingItems;

	// 리스트를 빠르게 찾기 위한 해쉬맵
	map<int32, TirdListPtr> tirdHashMap;
	map<int32, TifdListPtr> tifdHashMap;
	map<int32, PListPtr>	pairingHashMap;

	wstring info = L"DoubleClicked";
	wstring timeInfo = L"";
	string	startTimeInfo = "";

private:
	int32 screenWidth;
	int32 screenHeight;

	int startX = 0;
	int startY = 0;
	int32 tabWidth = 0;
	int32 tabHeight = 0;
	int32 logWidth = 0;
	int32 logHeight = 0;
	int32 logSize = 0;

	int32 pairingSize = 0;

private:
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

private:
	// Lock 부분 
	enum
	{
		USE_MODE = 0x0'000F
		, USE_ID = 0x0F'FFF0
		, WRITE = 0x0'0001
		, NONE = 0x0'0000
	};

	atomic<int32>			IsInfo = 0;
	shared_ptr<MapInfo>		mapInfo = nullptr;
	shared_ptr<InfoHandle>	infoHandle = nullptr;
};