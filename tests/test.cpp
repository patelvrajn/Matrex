#include "gtest/gtest.h"
#include "hello_world.hpp"

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

TEST(hello_world_test, test_one) { ASSERT_EQ(print_hello_world(), 0); }
