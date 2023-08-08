#pragma once
class DeadLock
{
public:
	void PushLog(string str);
	void SaveLog();

private:
	std::vector<string> logs;
	std::mutex m;
};

extern DeadLock* GDeadLock;