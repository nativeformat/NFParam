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
  python-pip \
  python-virtualenv \
  software-properties-common \
  wget

# Extra package repo for clang-format-4.0
sudo add-apt-repository -y ppa:ubuntu-mozilla-security/ppa 
sudo apt-get -q update
sudo apt-get install -y clang-format-4.0

# Update submodules
git submodule update --init --recursive

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
