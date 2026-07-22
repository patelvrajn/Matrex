#!/bin/bash -x

set -euo pipefail

make clean
bear --append -- make all
# Clang-tidy is completely broken for Matrex's repository - this will need fixing later.
# find . \( -iname '*.cpp' \) -print0 | xargs -0 clang-tidy -p . --checks='*' --fix
find . -iname '*.cpp' -o -iname '*.hpp' | clang-format --style=file -i --files=/dev/stdin

