#include "ThreadPoolBase.h"

namespace CNConcurrency
{
	class CRThreadAPI CThreadPool : protected CNConcurrency::ThreadPoolBase
	{
	public:
		// Rewrite constructor to account for multiple instances of thread pools
		explicit CThreadPool(int NumberOfThreads)
		{			
			if (NumberOfThreads != 0 && NumberOfThreads < SafeThreadCount)
			{
				StartPool(NumberOfThreads);
			}
			else
			{
				// Accuracy of this check is dependant on fixing issues with tracking instance of this class
				throw new CConcurrencyException;
			}
		}

		~CThreadPool()
		{
			do 
			{
				StopPool();

			} while (Threads.size() != 0);
		}

	public:

		template<class generic>
		auto AddTaskToQueue(generic task)->std::future<decltype(task())>;


	private:

		std::vector<CNConcurrency::CThread> Threads;
		std::condition_variable  EventVariable;

		std::mutex EventMutualExclusionZone;
		bool Stopping = false;

		std::queue<CTask> TaskQueue;



	public:

		void StartPool(std::size_t NumberOfThreads);

		void RunTask(CTask task);
		void StopTask(CTask task);

		void CheckThreads();
		void EnumerateThreads();
		void StopPool() noexcept;

	};

	template<class generic>
	inline auto CThreadPool::AddTaskToQueue(generic task) -> std::future<decltype(task())>
	{
		auto lockwrap = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));
		{
			std::unique_lock<std::mutex> AddTaskLock{ EventMutualExclusionZone };
			TaskQueue.emplace([=] {
				(*lockwrap)();
				});
		}

		EventVariable.notify_one();
		return lockwrap->get_future();

	}





}

