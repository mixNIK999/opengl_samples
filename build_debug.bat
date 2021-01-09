@ECHO ON

RMDIR /Q /S build_debug
MKDIR build_debug
PUSHD build_debug

REM conan install .. -s build_type=Debug
REM cmake .. -G "Visual Studio 15 2017 Win64"
REM cmake --build . --config Debug

conan install .. -s build_type=Debug
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Debug
