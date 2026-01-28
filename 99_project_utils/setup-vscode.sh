#!/bin/bash

echo "=========================================="
echo "Installing C++ Development Tools"
echo "=========================================="

echo "Updating package list..."
sudo apt-get update

echo ""
echo "Installing clang-format..."
sudo apt-get install -y clang-format

echo ""
echo "Installing clang-tidy (Clang analyzer)..."
sudo apt-get install -y clang-tidy

echo ""
echo "Installing cppcheck..."
sudo apt-get install -y cppcheck

echo ""
echo "Installing GDB (debugger)..."
sudo apt-get install -y gdb

echo ""
echo "=========================================="
echo "Verifying installations..."
echo "=========================================="
clang-format --version
clang-tidy --version
cppcheck --version
gdb --version

echo ""
echo "=========================================="
echo "Done! Restart VS Code."
echo "=========================================="
