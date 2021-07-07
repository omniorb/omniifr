REM To be executed in a Visual 2005 / C++ 8.0 environment

PUSHD %~dp0
DEL /S /Q build
MKDIR build
CD build
nmake ../Makefile
POPD
