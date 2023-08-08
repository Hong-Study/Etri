#pragma once

#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <iostream>
using namespace std;

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <thread>

#include "Types.h"
#include "Packet.h"
#include "Utils.h"