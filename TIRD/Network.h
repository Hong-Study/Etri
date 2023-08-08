#pragma once
class Network
{
#define BUFSIZE 512
public:
	Network();
	~Network();

public:
	void Connect(wstring ip, string id);
	void Connect(SOCKADDR_IN sockAddr);
	void Disconnet();
	void Recv();

	void	Send(BYTE* buf, int32 size);
	void	HandleRegister();
	void	HandleConfirm();
	double	calculateDistance(double lat1, double lon1, double lat2, double lon2);
	int32	GetTestDistance(double p1, double p2);
    
private:
	SOCKET _socket;
	SOCKADDR_IN _sockAddr;
	BYTE* _recvBuf;
	string id;
	thread* thread;
};

