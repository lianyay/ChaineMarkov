#!/bin/bash

echo "=== Nettoyage et Construction ==="
rm -rf build
mkdir build
cd build
cmake ..
make
echo "=== Exectuion du Programm ==="
./TI_301_PJT
