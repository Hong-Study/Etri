#include "pch.h"
#include "Network.h"
#include <thread>

Network::Network()
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == INVALID_SOCKET)
		cout << "socket error" << endl;

	ZeroMemory(&_sockAddr, sizeof(SOCKADDR_IN));

	_recvBuf = new BYTE[BUFSIZE];
	ZeroMemory(_recvBuf, BUFSIZE);
}

Network::~Network()
{
	Disconnet();
}

void Network::Connect(wstring ip, string id)
{
	this->id = id;
	SOCKADDR_IN sockAddr;
	sockAddr.sin_port = htons(PORT);
	sockAddr.sin_family = AF_INET;

	IN_ADDR address;
	::InetPtonW(AF_INET, IP, &address);
	sockAddr.sin_addr = address;

	Connect(sockAddr);
}

void Network::Connect(SOCKADDR_IN sockAddr)
{
	_sockAddr = sockAddr;
	if (connect(_socket, reinterpret_cast<SOCKADDR*>(&_sockAddr), sizeof(_sockAddr)) == SOCKET_ERROR)
		cout << "connect Failed" << endl;
	else
		cout << "ConnectSuccess" << endl;
}

void Network::Disconnet()
{
	closesocket(_socket);
}

void Network::Recv()
{
	int32 retVal = 0;
	while (true)
	{
		retVal = recv(_socket, reinterpret_cast<char*>(_recvBuf), BUFSIZE, 0);
		cout << "RecvSize : " << retVal << endl;
		if (retVal <= 0)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)
				continue;
			cout << "Error : " << errorCode << endl;
			break;
		}

		PktHead* head = reinterpret_cast<PktHead*>(_recvBuf);

		switch (head->command)
		{
		case CommandType::CmdType_Reg_Request:
			HandleRegister();
			break;
		case CommandType::CmdType_Reg_Confirm:
			HandleConfirm();
			break;
		}
	}
}

void Network::GetTime()
{
	auto now = std::chrono::system_clock::now();

	// 현재 시간을 std::time_t 타입으로 변환합니다.
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// 스레드 안전한 버전인 std::localtime_s 함수를 사용하여 날짜와 시간 정보를 분리합니다.
	localtime_s(&local_Time, &now_time);
	
}

void Network::Send(BYTE* buf, int32 size)
{
	int32 retVal = send(_socket, reinterpret_cast<char*>(buf), size, 0);
	if (retVal <= 0)
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSAEWOULDBLOCK)
		{
			cout << "SendFailed" << endl;
			Disconnet();
		}
	}
	delete[] buf;
}

void Network::HandleRegister()
{
	cout << "Register" << endl;

	GetTime();
	StTifdData data;
	ZeroMemory(&data, sizeof(StTifdData));
	data.nTifdVer[0] = 2;
	data.nTifdVer[1] = 3;
	data.nTifdVer[2] = 4;
	data.nTifdVer[3] = 5;

	strcpy_s(data.deviceId, 12, id.c_str());
	data.deviceId[11] = '\0';

	data.nLoraVer[0] = 2;
	data.nLoraVer[1] = 3;
	data.nLoraVer[2] = 4;
	data.nLoraVer[3] = 5;

	data.trainNo = 203;
	data.trainLength = 1234;
	data.trainStatus = 0;
	data.lat = 0.4f;
	data.lon = 0.f;
	data.distance = 0;
	data.speed = 21;

	data.stTime.hour = local_Time.tm_hour;
	data.stTime.min = local_Time.tm_min;
	data.stTime.sec = local_Time.tm_sec;

	BYTE* buf = MakeSendBuffer(data, CmdType_Reg_Request, 0, Device::DeviceTIFD);

	Send(buf, HEAD_SIZE + sizeof(StTifdData));
}

void Network::HandleConfirm()
{
	cout << "Confirm" << endl;

	thread = new std::thread([=]() {
		float latitude = 0.f;
		float longitude = 0.f;
		uint64 tick = GetTickCount64();
		while (true)
		{
			uint64 now = GetTickCount64();
			if (now - tick > 1000)
			{
				GetTime();

				StTifdData data;
				ZeroMemory(&data, sizeof(StTifdData));
				data.nTifdVer[0] = 2;
				data.nTifdVer[1] = 3;
				data.nTifdVer[2] = 4;
				data.nTifdVer[3] = 5;

				strcpy_s(data.deviceId, 12, id.c_str());
				data.deviceId[11] = '\0';

				data.nLoraVer[0] = 2;
				data.nLoraVer[1] = 3;
				data.nLoraVer[2] = 4;
				data.nLoraVer[3] = 5;

				data.trainNo = 203;
				data.trainLength = 1234;
				data.trainStatus = 0;
				data.lat = latitude;
				data.lon = longitude;
				data.speed = 21;
				data.distance = 0;

				data.stTime.hour = local_Time.tm_hour;
				data.stTime.min = local_Time.tm_min;
				data.stTime.sec = local_Time.tm_sec;

				BYTE* buf = MakeSendBuffer(data, CmdType_Tifd_Info, 0, Device::DeviceTIFD);
				Send(buf, HEAD_SIZE + sizeof(StTifdData));
				tick = now;
				latitude += 0.0001f;
				longitude += 0.0001f;
			}
		}
		});
}

double Network::calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
	double pi = std::acos(-1);  // 3.14159265358979323846
	double earthRadius = 6371;  // 지구의 반지름 (단위: km)

	// 위도와 경도를 라디안으로 변환
	double lat1Rad = lat1 * pi / 180.0;
	double lon1Rad = lon1 * pi / 180.0;
	double lat2Rad = lat2 * pi / 180.0;
	double lon2Rad = lon2 * pi / 180.0;

	// 중심각 계산
	double deltaLat = lat2Rad - lat1Rad;
	double deltaLon = lon2Rad - lon1Rad;
	double centralAngle = 2 * asin(sqrt(pow(sin(deltaLat / 2), 2) + cos(lat1Rad) * cos(lat2Rad) * pow(sin(deltaLon / 2), 2)));

	// 거리 계산
	double distance = (earthRadius * centralAngle) * 1000;
	int normal = static_cast<int32>(distance);
	return abs(normal);
}

int32 Network::GetTestDistance(double p1, double p2)
{
	double distance = (p1 - p2) * 1000;
	int normal = static_cast<int32>(distance);

	return abs(normal);
}