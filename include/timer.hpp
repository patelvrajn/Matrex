#include <chrono>

class Timer {
 public:
  Timer();

  uint64_t stop();

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
