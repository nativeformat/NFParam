#!/bin/bash

# Exit on any non-zero status
set -e

# Set up virtualenv
virtualenv nfparam_env
source nfparam_env/bin/activate

# Install Python Packages
pip install pyyaml
pip install flake8
pip install cmakelint
pip install requests

# Execute our python build tools
if [ -n "$BUILD_IOS" ]; then
    python ci/ios.py "$@"
else
    python ci/osx.py "$@"
fi
