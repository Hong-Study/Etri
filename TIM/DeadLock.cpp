#include "pch.h"
#include "DeadLock.h"

DeadLock* GDeadLock = new DeadLock;

void DeadLock::PushLog(string str)
{
    std::lock_guard guard(m);

    logs.push_back(str);
}

void DeadLock::SaveLog()
{
	static bool check = true;

	if (check)
	{
        string createPath = "../Logs/Logs.txt";
        std::filesystem::path path(createPath);

        if (std::filesystem::exists(path.parent_path()) == false)
        {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream out(createPath);

        for (auto log : logs)
        {
            out << log;
        }

        out.close();
		check = false;
	}
}
