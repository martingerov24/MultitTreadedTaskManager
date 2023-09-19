#include "TaskSystem.h"
#include <cassert>
#if defined(_WIN32) || defined(_WIN64)
#define USE_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef LoadLibrary
#else
#include <dlfcn.h>
#endif

namespace TaskSystem {

TaskSystemExecutor::TaskSystemExecutor(int threadCount)
: tm(ThreadManager::GetInstance())
, m_priority_queue(PriorityComparator()) 
{
    m_loopingThread = std::thread(&TaskSystemExecutor::executeTasks, std::ref(*this));
}

void TaskSystemExecutor::executeTasks() {
    while(stopFlag == false) {

    }
}

TaskSystemExecutor* TaskSystemExecutor::self = nullptr;

TaskSystemExecutor &TaskSystemExecutor::GetInstance() {
    return *self;
}

void TaskSystemExecutor::Init(int threadCount) {
    delete self;
    ThreadManager::Init(threadCount);
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

std::unique_ptr<Task>* TaskSystemExecutor::findTaskById(const TaskID taskId) {
    const auto& container = m_priority_queue.c;
    for (auto& task_ptr : container) {
        const Task& task = *task_ptr;
        if (task.taskId == taskId) {
            return &task_ptr;
        }
    }
    return nullptr;
}

const TaskID TaskSystemExecutor::ScheduleTask(std::unique_ptr<Task> task, int priority) {
    task->m_priority = priority;
    m_priority_queue.emplace(task);
    return task.get()->getTaskId();
}

bool TaskSystemExecutor::WaitForTask(TaskID taskId) {
    std::unique_ptr<Task>* taskPtr = findTaskById(taskId);
    if (taskPtr == nullptr) {
        return false;
    }
    
    std::unique_ptr<Task>& task = *taskPtr;
    std::unique_ptr<Executor> exec(m_executors[task->GetExecutorName()](std::move(task)));
    exec->waitForCompletion();
    return true;
}

void TaskSystemExecutor::OnTaskCompleted(TaskID task, std::function<void(TaskID)> &&callback) {
    callback(task);
}
void TaskSystemExecutor::Register(const std::string &executorName, ExecutorConstructor constructor) {
    m_executors[executorName] = constructor;
}


};
