#!/bin/bash

# Remove existing environment if it exists
conda env remove -n lambdamail

# Create new environment
conda env create -f environment.yml

# Activate environment
conda activate lambdamail

# Build and install restclient-cpp manually in the conda environment
git clone https://github.com/mrtazz/restclient-cpp.git
cd restclient-cpp
./autogen.sh
./configure --prefix=$CONDA_PREFIX
make
make install
cd ..
rm -rf restclient-cpp

echo "Setup complete! You can now build your project:"
echo "mkdir build"
echo "cd build"
echo "cmake .."
echo "make"
