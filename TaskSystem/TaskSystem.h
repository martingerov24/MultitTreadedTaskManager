#pragma once

#include "Task.h"
#include "Executor.h"
#include <map>
#include <functional>
#include <random>
#include <queue>

namespace TaskSystem {

class ThreadManager;

class TaskSystemExecutor {
    TaskSystemExecutor(int threadCount);
public:
    TaskSystemExecutor(const TaskSystemExecutor &) = delete;
    TaskSystemExecutor &operator=(const TaskSystemExecutor &) = delete;

    static void Init(int threadCount);
    static TaskSystemExecutor &GetInstance();

    void WaitForTask(TaskID task);
    TaskID ScheduleTask(std::unique_ptr<Task> task, int priority);
    void OnTaskCompleted(TaskID task, std::function<void(TaskID)> &&callback);
    bool LoadLibrary(const std::string &path);
    void Register(const std::string &executorName, ExecutorConstructor constructor);
private:
    static TaskSystemExecutor *self;
    std::map<std::string, ExecutorConstructor> m_executors;
    std::priority_queue<std::unique_ptr<Task>, std::vector<std::unique_ptr<Task>>, PriorityComparator> m_priority_queue;
    ThreadManager& tm;
};

};
