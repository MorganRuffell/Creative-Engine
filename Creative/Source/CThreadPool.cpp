#include "CThreadPool.h"

using namespace CNConcurrency;

void CThreadPool::StartPool(std::size_t NumberOfThreads)
{
	if (NumberOfThreads <= SafeThreadCount)
	{
		for (auto x = 0u; x < NumberOfThreads; x++)
		{
			EnumerateThreads();
		}
	}
	else
	{
		StopPool();
		throw new CConcurrencyException;
	}
	
}

void CNConcurrency::CThreadPool::RunTask(CTask task)
{
}

void CNConcurrency::CThreadPool::CheckThreads()
{
	for (int i = 0; i < Threads.size(); i++)
	{
		if (Threads.at(i).get_id() == std::jthread::id{})
		{
			throw new CConcurrencyException;
		}
	}
}

void CNConcurrency::CThreadPool::EnumerateThreads()
{
	Threads.emplace_back([=] {

		while (true)
		{
			CTask newtask;

			{
				std::unique_lock<std::mutex> lock
				{
					EventMutualExclusionZone
				};

				EventVariable.wait(lock, [&] {
					return Stopping || !TaskQueue.empty();
					});

				if (Stopping && TaskQueue.empty())
					break;

				newtask = std::move(TaskQueue.front());
				TaskQueue.pop();
			}

			newtask();
		}

	});
}

void CThreadPool::StopPool() noexcept
{
	{
		std::unique_lock<std::mutex> lock{ EventMutualExclusionZone };
		Stopping = true;
	}

	EventVariable.notify_all();


	for (auto& thread : Threads)
		thread.join();
}



