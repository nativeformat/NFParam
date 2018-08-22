#!/bin/bash

# Exit on any non-zero status
set -e

# Install system dependencies
sudo apt-get install -y --no-install-recommends apt-utils \
  clang-format-3.8 \
  clang-3.8 \
  git \
  libc++-dev \
  python-pip \
  python-virtualenv \
  wget

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
python ci/linux.py
