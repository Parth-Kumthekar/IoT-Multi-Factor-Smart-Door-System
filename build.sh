#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "--- 🧹 Cleaning previous build ---"
rm -rf build

echo "--- 📁 Creating build directory ---"
mkdir build
cd build

echo "--- ⚙️ Running CMake ---"
cmake ..

echo "--- 🔨 Building executable ---"
# -j$(nproc) tells make to use all available CPU cores for a faster build
make -j$(nproc)

echo "--- ✅ Build Complete! ---"
