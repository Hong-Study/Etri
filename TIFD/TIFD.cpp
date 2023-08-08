#include "pch.h"
#include <thread>
#include <memory>
#include <random>
#include <string>
#include "Network.h"

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		cout << "ERROR" << endl;

	vector<thread> threads;
	while (1)
	{
		char id[12];
		cin >> id;

		threads.push_back(thread([=]() {
			shared_ptr<Network> network = make_shared<Network>();
			network->Connect(IP, id);

			network->Recv();
			}));
	}

	for (thread& t : threads)
	{
		t.join();
	}
	
}

