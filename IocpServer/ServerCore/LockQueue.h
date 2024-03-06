#pragma once

template<typename T>
class LockQueue
{
public:
	inline void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);

		_q.push(value);
	}

	inline bool Pop(OUT T& value)
	{
		lock_guard<mutex> lock(_mutex);

		if (_q.empty())
			return false;

		value = _q.front();
		_q.pop();
		return true;
	}

	inline void PopAll(OUT vector<T>& values)
	{
		lock_guard<mutex> lock(_mutex);

		while (_q.empty() == false)
		{
			values.push_back(_q.front());
			_q.pop();
		}
	}

	inline void Clear()
	{
		lock_guard<mutex> lock(_mutex);

		_q = queue<T>();
	}

private:
	mutex _mutex;
	queue<T> _q;
};