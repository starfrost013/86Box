@echo off
cmake -DCMAKE_BUILD_TYPE=%1 -B build\ -S .
cmake --build build\
copy build\src\86Box.exe install\86Box.exe