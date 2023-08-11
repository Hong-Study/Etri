#pragma once

#define WIN32_LEAN_AND_MEAN 
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "python3.lib")
#pragma comment(lib, "python38.lib")
#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <CommCtrl.h>
#include <iostream>

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#include <ole2.h>
#include <gdiplus.h>

#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <mutex>
#include <ctime>
#include <algorithm>
#include <string>
#include <chrono>
#include <tchar.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <stack>
#include <Python.h>
#include <memory>

#include "Types.h"
#include "Macro.h"
#include "Information.h"
#include "Lock.h"
#include "Utils.h"
#include "SendBuffer.h"
#include "resource.h"
#include "RecvBuffer.h"
#include "TLS.h"
#include "Global.h"