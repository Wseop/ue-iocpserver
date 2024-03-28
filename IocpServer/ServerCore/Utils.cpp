#include "pch.h"
#include "Utils.h"

string Utils::WStrToStr(const wstring& wstr)
{
	int32 len = WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, NULL, 0, NULL, NULL);
	
	string str;
	str.resize(len);
	WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, str.data(), len, NULL, NULL);
	
	return str;
}
