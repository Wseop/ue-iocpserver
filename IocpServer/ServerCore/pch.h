#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib, "Spdlog\\Debug\\spdlogd.lib")
#else
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#pragma comment(lib, "Spdlog\\Release\\spdlog.lib")
#endif // _DEBUG

#include "CorePch.h"