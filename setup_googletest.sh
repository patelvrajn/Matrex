git clone --depth=1 -b main -q https://github.com/google/googletest.git ./googletest
mkdir -p ./googletest/build
cd ./googletest/build
sudo cmake .. && sudo make && sudo make install
cd ../../
sudo rm -rf googletest

