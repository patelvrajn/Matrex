#pragma once

#include <chrono>

constexpr uint64_t NANOSECONDS_IN_SECOND      = 1e9;
constexpr uint64_t NANOSECONDS_IN_MILLISECOND = 1e6;

class Timer
{
  public:

    Timer();

    void     start();
    uint64_t elapsed();
    bool     is_search_time_expired(uint64_t time_remaining_constraint,
                                    uint64_t time_increment_constraint);

  private:

    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
