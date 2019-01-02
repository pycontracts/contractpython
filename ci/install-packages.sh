#! /bin/bash

# For g++-8
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

# We need a more recent meson too
sudo add-apt-repository ppa:jonathonf/meson -y

sudo apt-get update

# not really needed?
#sudo apt-get dist-upgrade -y

sudo apt-get install meson build-essential git g++-8 python3-numpy libgtest-dev libgflags-dev libboost-program-options-dev libpython3-dev libboost-python-dev pkg-config python3-pytest cmake clang-6.0 libgmp-dev gdb net-tools libssl-dev libgoogle-glog-dev -y
