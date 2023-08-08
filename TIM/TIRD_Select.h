#pragma once

class TIRD_Select
{
	SINGLETON(TIRD_Select)
public:
	void Init();
	void Update();
	void Disconnect();

private:
	SOCKET _listenSocket = INVALID_SOCKET;
	SOCKADDR_IN _sockAddr;
	fd_set _fds;
	struct timeval _cv;
};

