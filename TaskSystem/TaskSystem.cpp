#include "TaskSystem.h"

#include <cassert>
#include <chrono>
#include <climits>
#include <random>
#if defined(_WIN32) || defined(_WIN64)
#define USE_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef LoadLibrary
#else
#include <dlfcn.h>
#endif

namespace TaskSystem {

TaskSystemExecutor* TaskSystemExecutor::self = nullptr;

TaskSystemExecutor &TaskSystemExecutor::GetInstance() {
    return *self;
}

TaskID::TaskID(const int32_t priority)
: m_priority(priority) {
    std::random_device rd;
    std::mt19937_64 rng;
    
    rng.seed(rd());
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::uniform_int_distribution<unsigned long> distribution(0, ULONG_MAX);
    unsigned long randomNum = distribution(rng);

    m_uniqueID = (timestamp << 32) | randomNum;
}

void TaskSystemExecutor::Init(int threadCount) {
    delete self;
    self = new TaskSystemExecutor(threadCount);
}

bool TaskSystemExecutor::LoadLibrary(const std::string &path) {
#ifdef USE_WIN
    HMODULE handle = LoadLibraryA(path.c_str());
#else
    void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        const char* error = dlerror();
        printf("the error is %s\n", error);
    } 
#endif
    assert(handle);
    if (handle) {
        OnLibraryInitPtr initLib =
#ifdef USE_WIN
            (OnLibraryInitPtr)GetProcAddress(handle, "OnLibraryInit");
#else
            (OnLibraryInitPtr)dlsym(handle, "OnLibraryInit");
#endif
        assert(initLib);
        if (initLib) {
            initLib(*this);
            printf("Initialized [%s] executor\n", path.c_str());
            return true;
        }
    }
    return false;
}


TaskID TaskSystemExecutor::ScheduleTask(std::unique_ptr<Task> task, int priority) {
    std::unique_ptr<Executor> exec(m_executors[task->GetExecutorName()](std::move(task)));
    TaskID currentTaskId(priority);
    
    while (exec->ExecuteStep(0, 1) != Executor::ExecStatus::ES_Stop)
        ;

    return currentTaskId;
}

void TaskSystemExecutor::WaitForTask(TaskID task) {
    return;
}
void TaskSystemExecutor::OnTaskCompleted(TaskID task, std::function<void(TaskID)> &&callback) {
    callback(task);
}
void TaskSystemExecutor::Register(const std::string &executorName, ExecutorConstructor constructor) {
    executors[executorName] = constructor;
}


};
