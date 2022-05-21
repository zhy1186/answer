#!/bin/bash
mkdir bin
mkdir build && cd build
cmake3 -S ../ -B .
make
cd ..
cp ${PWD}/build/answer/client ${PWD}/bin/cpp_client
cp ${PWD}/build/answer/server ${PWD}/bin/cpp_server
