#pragma once

#include "Task.h"
#include <mutex>
#include <queue>
#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>


namespace TaskSystem {
class ThreadManager;

struct Executor {
    enum ExecStatus {
        ES_Continue, ES_Stop
    };
 
    Executor(std::unique_ptr<Task> taskToExecute) : task(std::move(taskToExecute)) {}
    virtual ExecStatus ExecuteStep(int threadIndex, int threadCount) = 0;
        
    inline void runOn(ThreadManager &tm);
    virtual ~Executor() {}

    std::unique_ptr<Task> task;
};

struct CallbackFunctor {

    typedef std::function<void(int, int)> Signature;
	Signature callback;

	CallbackFunctor(Signature callback)
		: callback(callback)
	{}

	void run(int threadIndex, int threadCount) {
		callback(threadIndex, threadCount);
	}
};

/**
 * @brief Type of the needed function in the dynamic library for executor creation
 *
 */
typedef Executor*(*ExecutorConstructor)(std::unique_ptr<Task> taskToExecute);
struct TaskSystemExecutor;


/// Non re-entrant task runner
class ThreadManager {
	explicit ThreadManager(int threadCount)
		: count(threadCount)
	{}
public:
	ThreadManager(const ThreadManager &) = delete;
	ThreadManager& operator=(const ThreadManager &) = delete;

    static void Init(int threadCount);

    static ThreadManager &GetInstance();

	/// Start up all threads, must be called before @runThreads is called
	void start();

	/// Schedule the task to be run by the threads and return immediatelly
	/// This function could return before the threads have actually started running the task!
	void runThreadsNoWait(TaskSystem::Executor &task);

	/// Start a task on all threads and wait for all of them to finish
	/// @param task - the task to run on all threads
	void runThreads(TaskSystem::Executor &task);

	/// Blocking wait for all threads to exit, does not interrupt any running Task
	void stop();

	/// Get the number of worker threads
	int getThreadCount() const;

private:
    static ThreadManager *self;
	/// The entry point for all of the threads
	/// @param threadIndex - the 0 based index of the thread
	void threadBase(volatile int threadIndex);

	/// Check if all elements in @currentTask are nullptr
	/// @return true if at least one task is not nullptr, false otherwise
	bool unlockedAllTasksDone() const;

	int count = -1; ///< The number of threads
	std::vector<std::thread> threads; ///< The thread handles

	bool running = false; ///< Flag indicating if threads should quit

	/// The current task for each thread, always must be the same element
	/// Used to track if thread has finished working
	std::vector<Executor *> currentTask;
	std::mutex workMtx; ///< Mutex protecting @currentTask and @running

	/// The event used to signal workers when new task is available
	/// Also used by workers to signal when task is finished
	std::condition_variable workEvent;
};


inline void Executor::runOn(ThreadManager &tm) {
    tm.runThreads(*this);
}

};

/**
 * @brief Each executor library must define OnLibraryInit function that will be called once on load
 *        It should be used to register the executor in the TaskSystemExecutor
 *
 */
typedef void (*OnLibraryInitPtr)(TaskSystem::TaskSystemExecutor &);

#if defined(_WIN32) || defined(_WIN64)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define IMPLEMENT_ON_INIT() extern "C" DLL_EXPORT void OnLibraryInit(TaskSystem::TaskSystemExecutor &ts)
