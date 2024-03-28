#pragma once

#include <random>

class Utils
{
public:
	template<typename T>
	static inline T GetRandom(T min, T max)
	{
		random_device randomDevice;
		mt19937 generator(randomDevice());

		if constexpr (is_integral_v<T>)
		{
			uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}
		else
		{
			uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

	static string WStrToStr(const wstring& wstr);
};

