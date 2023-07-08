#pragma once

#include <string>
#include <memory>
#include <ostream>
#include <optional>
#include <functional>

namespace TaskSystem {

struct TaskID {
    TaskID();
    inline const uint64_t get() const { return m_uniqueID; }
private:
    uint64_t m_uniqueID;
};

struct PriorityComparator;

struct Task {
    virtual std::optional<int>         GetIntParam(const std::string &name)     const { return std::nullopt; }
    virtual std::optional<std::string> GetStringParam(const std::string &name)  const { return std::nullopt; }
    virtual std::optional<double>      GetDoubleParam(const std::string &name)  const { return std::nullopt; }
    virtual std::optional<void*>       GetAnyParam(const std::string &name)     const { return std::nullopt; }
    virtual std::string                GetExecutorName() const = 0;

    inline bool operator< (const Task& other) const { return m_priority <  other.m_priority; }
    inline bool operator> (const Task& other) const { return m_priority >  other.m_priority; }
    inline bool operator==(const Task& other) const { return m_priority == other.m_priority; }
    inline bool operator!=(const Task& other) const { return m_priority != other.m_priority; }
    friend std::ostream& operator<<(std::ostream& os, const Task& task) {
        os << "Priority: " << task.m_priority;
        return os;
    }
    friend struct PriorityComparator;
    friend class TaskSystemExecutor;
private:
    int32_t m_priority;
};

struct PriorityComparator {
    bool operator()(const std::unique_ptr<Task>& task1, const std::unique_ptr<Task>& task2) const {
        return task1->m_priority > task2->m_priority;
    }
};

};