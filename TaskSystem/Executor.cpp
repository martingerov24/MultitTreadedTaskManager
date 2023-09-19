#include "Executor.h"
#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>
#include <functional>

namespace TaskSystem {

void Executor::waitForCompletion() {
    std::unique_lock<std::mutex> lock(completionMutex);
    completionCV.wait(lock, [this]() { return isTaskCompleted; });
}

void Executor::markTaskCompleted() {
    std::lock_guard<std::mutex> lock(completionMutex);
    isTaskCompleted = true;
    completionCV.notify_all();
}

ThreadManager* ThreadManager::self = nullptr;

ThreadManager &ThreadManager::GetInstance() {
    return *self;
}

void ThreadManager::Init(int threadCount) {
    delete self;
    self = new ThreadManager(threadCount);
}

void ThreadManager::start() {
    assert(count > 0 && "Task count must be positive");
    assert(threads.size() == 0 && "Already started");

    running = true;
    currentTask.resize(count, nullptr);

    threads.reserve(count);
    for (int c = 0; c < count; c++) {
        threads.emplace_back(&ThreadManager::threadBase, this, c);
    }
    // does not block to wait for threads to start, if they are delayed @runThreads will make sure to wait for them
}

void ThreadManager::runThreadsNoWait(TaskSystem::Executor &task) {
    {
        std::lock_guard<std::mutex> lock(workMtx);
        assert(unlockedAllTasksDone() && "Already working");
        for (int c = 0; c < int(currentTask.size()); c++) {
            currentTask[c] = &task;
        }
    }

    workEvent.notify_all();
}

void ThreadManager::runThreads(TaskSystem::Executor &task) {
    runThreadsNoWait(task);

    // block until task is complete
    if (!unlockedAllTasksDone()) {
        std::unique_lock<std::mutex> lock(workMtx);
        if (!unlockedAllTasksDone()) {
            workEvent.wait(lock, [this]() {
                return unlockedAllTasksDone();
            });
        }
    }
}

void ThreadManager::stop() {
    assert(running && "Can't stop if not running");

    {
        std::lock_guard<std::mutex> lock(workMtx);
        running = false;
    }

    workEvent.notify_all();

    // joining threads will implicitly wait for all of them to finish current task
    for (int c = 0; c < int(threads.size()); c++) {
        threads[c].join();
    }

    threads.clear();
    currentTask.clear();
}

int ThreadManager::getThreadCount() const {
    return int(threads.size());
}

void ThreadManager::threadBase(volatile int threadIndex) {
    while (true) {

        TaskSystem::Executor *toExecute = nullptr;
        if (running) {
            std::unique_lock<std::mutex> lock(workMtx);

            if (running && currentTask[threadIndex] == nullptr) {
                workEvent.wait(lock, [this, threadIndex]() {
                    return currentTask[threadIndex] != nullptr || !running;
                });
            }

            // just copy, and do not clear the value in @currentTask
            // it is used to signal the task is completed
            toExecute = currentTask[threadIndex];
        }

        if (!running) {
            return;
        }

        assert(toExecute);

        toExecute->ExecuteStep(threadIndex, int(threads.size()));
        {
            std::lock_guard<std::mutex> lock(workMtx);
            currentTask[threadIndex] = nullptr;
        }

        // Since start and finish share an event this must wake all threads
        // to make sure the caller is awoken and not only some other worker
        workEvent.notify_all();
    }
}
bool ThreadManager::unlockedAllTasksDone() const {
    for (int c = 0; c < currentTask.size(); c++) {
        if (currentTask[c]) {
            return false;
        }
    }
    return true;
}

};