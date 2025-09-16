#include "timer.hpp"

Timer::Timer() : m_start(std::chrono::high_resolution_clock::now()) {}

uint64_t Timer::stop() {
  auto end = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start)
      .count();
}
