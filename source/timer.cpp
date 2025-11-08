#include "timer.hpp"

Timer::Timer() : m_start(std::chrono::high_resolution_clock::now()) {}

void Timer::start() { m_start = std::chrono::high_resolution_clock::now(); }

uint64_t Timer::elapsed() {
  auto now = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_start)
      .count();
}

bool Timer::is_search_time_expired(uint64_t time_remaining_constraint,
                                   uint64_t time_increment_constraint) {
  // 1e6 converts milliseconds to nanoseconds, formula for time per move is from
  // CPW.
  return (elapsed() >= (((time_remaining_constraint * 1e6) / 20) +
                        ((time_increment_constraint * 1e6) / 2)));
}
