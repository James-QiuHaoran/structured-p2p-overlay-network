@echo off
md build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -A x64 -T host=x64
cmake --build .