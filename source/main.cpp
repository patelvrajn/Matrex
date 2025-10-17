#include <iostream>

#include "uci.hpp"

/*******************************************************************************
    Only the entry point (main) function should be in this file because this
    file is excluded from testing.
*******************************************************************************/

int main(void) {
  UCI uci;

  uci.loop();

  return 0;
}
