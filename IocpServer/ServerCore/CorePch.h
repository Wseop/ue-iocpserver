#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

#include <Windows.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <functional>
#include <format>
using namespace std;

#include "Types.h"
#include "LockQueue.h"

#include "CoreGlobal.h"
#include "CoreTLS.h"

#include "SocketUtils.h"
#include "NetAddress.h"

#include "spdlog/spdlog.h"