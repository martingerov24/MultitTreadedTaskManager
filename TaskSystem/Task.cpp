#include "Task.h"

#include <chrono>
#include <climits>
#include <random>

namespace TaskSystem {

TaskID::TaskID() {
    std::random_device rd;
    std::mt19937_64 rng;
    
    rng.seed(rd());
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::uniform_int_distribution<unsigned long> distribution(0, ULONG_MAX);
    unsigned long randomNum = distribution(rng);

    m_uniqueID = (timestamp << 32) | randomNum;
}

};
