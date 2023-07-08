#pragma once

#include <string>
#include <optional>

namespace TaskSystem {

/**
 * @brief Base class providing arguments for Executor
 *
 */
struct Task {
    struct ID {
        ID(const int32_t priority);
        inline const uint64_t get() const { return m_uniqueID;}
        inline bool operator< (const ID& other) const { return m_priority <  other.m_priority; }
        inline bool operator> (const ID& other) const { return m_priority >  other.m_priority; }
        inline bool operator==(const ID& other) const { return m_priority == other.m_priority; }
        inline bool operator!=(const ID& other) const { return m_priority != other.m_priority; }
    private:
        uint64_t m_uniqueID;
        const int32_t m_priority;
    };

    virtual std::optional<int> GetIntParam(const std::string &name) const { return std::nullopt; }
    virtual std::optional<std::string> GetStringParam(const std::string &name) const { return std::nullopt; }
    virtual std::optional<double> GetDoubleParam(const std::string &name) const { return std::nullopt; }
    virtual std::optional<void*> GetAnyParam(const std::string &name) const { return std::nullopt; }
    virtual std::string GetExecutorName() const = 0;

    virtual ~Task() {}
};

};