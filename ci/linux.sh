#!/bin/bash

# Exit on any non-zero status
set -e

# Install system dependencies
sudo apt-get -q update
sudo apt-get install -y --no-install-recommends apt-utils \
  clang-3.8 \
  git \
  lcov \
  libc++-dev \
  ninja-build \
  python-pip \
  python-software-properties \
  python-virtualenv \
  software-properties-common \
  unzip \
  wget

# Extra repo for clang-format-4.0
sudo add-apt-repository -y ppa:ubuntu-mozilla-security/ppa 

# Extra repo for gcc-4.9 so we don't have to use 4.8
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test

sudo apt-get -q update
sudo apt-get install -y --no-install-recommends \
  clang-format-4.0 \
  gcc-4.9 \
  g++-4.9

# Update submodules
git submodule update --init --recursive

# install cmake 3.6.x
wget --no-check-certificate https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.sh
chmod +x cmake-3.6.3-Linux-x86_64.sh
sudo sh cmake-3.6.3-Linux-x86_64.sh --prefix=/usr/local --exclude-subdir

export CC=clang-3.8
export CXX=clang++-3.8

# Set up virtualenv
virtualenv nfparam_env
. nfparam_env/bin/activate

# Install Python Packages
pip install pyyaml \
  flake8 \
  cmakelint

# Execute our python build tools
python ci/linux.py "$@"
