#pragma once

#include <string>
#include <optional>

struct Task {
    virtual std::optional<int> GetIntParam(const std::string &name) const { return std::nullopt; }
    virtual std::optional<std::string> GetStringParam(const std::string &name) const { return std::nullopt; }
    virtual std::optional<double> GetDoubleParam(const std::string &name) const { return std::nullopt; }
    virtual std::optional<void*> GetAnyParam(const std::string &name) const { return std::nullopt; }

    virtual std::string GetExecutorName() const = 0;

    virtual ~Task() {}
};