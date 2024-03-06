#pragma once

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#endif // _DEBUG

#include "CorePch.h"