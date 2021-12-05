#pragma once

#include <thread>
#include <utility>


namespace CNConcurrency
{
	struct CThread : public std::jthread
	{
	public:

		using jthread::jthread;
		using jthread::operator=;

		//CThread()
		//{
		//	if (jthread::joinable())
		//	{
		//		jthread::join();
		//	}
		//}

		//
		/*	~CThread()
		{
			if (jthread::joinable())
			{
				jthread::join();
			}
		}*/
		//Inactive will call std::jthread instead of our implementation.

		static int CThreadSystemMaxCount()
		{
			return _Thrd_hardware_concurrency();
		}


		//If this breaks do this -- https://stackoverflow.com/questions/49789325/guarded-thread-class-move-assignment-operator-error

	};
};

