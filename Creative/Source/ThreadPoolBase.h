#pragma once

#include <iostream>
#include <condition_variable>
#include <vector>
#include <future>
#include "CThread.h"
#include <functional>
#include <queue>
#include "CreativeMacros.h"
#include "CException.h"
#include <assert.h>

namespace CNConcurrency
{
	class CRThreadAPI ThreadPoolBase
	{
	protected:

		using CTask = std::function<void()>;

		ThreadPoolBase()
		{
			SafeThreadCount = std::jthread::hardware_concurrency() - 3;

			if (MaximumTotalThreads == 0 || SafeThreadCount == 0 || SafeThreadCount == MaximumTotalThreads)
			{
				throw new CConcurrencyException;
			}
		}

		~ThreadPoolBase()
		{
			

		}

	public:

		int FetchSafeThreadCount()
		{
			return SafeThreadCount;
		}

	private:
		//WARNING - THIS IS FOR ENGINE DEV USE ONLY -- RESTRICTIONS ON MAXIMUM THREAD COUNT ARE THERE TO PROTECT USERS COMPUTERS.
		//only use if you KNOW what you are doing!
		int IncreaseSafeThreadCount(int desiredIncrease)
		{
			assert(SafeThreadCount + desiredIncrease < MaximumTotalThreads);
			//assert(desiredIncrease < 3);

			if (SafeThreadCount + desiredIncrease != MaximumTotalThreads)
			{
				SafeThreadCount + desiredIncrease;
			}
			else
			{
				throw new CConcurrencyException;
			}
		}




	protected:

		const int MaximumTotalThreads = std::jthread::hardware_concurrency();
		static int SafeThreadCount; 
	};

}



