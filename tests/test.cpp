#include "gtest/gtest.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  std::cout << "RUNNING TESTS ..." << std::endl;
  int ret{RUN_ALL_TESTS()};
  if (!ret) {
    std::cout << "SUCCESS!!!" << std::endl;
    return 0;
  } else {
    std::cout << "FAILED." << std::endl;
    return 1;
  }
}
