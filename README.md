# OmniIFR2

Interface Repository for omniORB, a CORBA ORB for C++ and Python.

Old official repository for previous versions up to 2.pre.1:
https://sourceforge.net/projects/omniifr/

See old README for more information about previous versions.

## How to build on Windows with Visual C++

Tested with:
- Visual 2005 SP6 (C++ version 8)
- OmniORB 4.1.7 (2.pre.1) / 4.2.3 (2.pre.2)
- Python 1.5.2 (2.pre.1) / 3.5.3 (2.pre.2)

Build steps:
- Open a command prompt within the Visual build environment
- Go to the `visual` subfolder
- Run the `build.bat` script

OmniIFR.exe is built in release mode in `visual\Release` sub-folder.

## How to build on Windows with MSYS2

Tested 2.pre.2 with:
- MSYS2 core (updated 2018-09-19)
- OmniORB 4.2.3
- Python 3.5.3

Build steps:
- Open a MSYS2 command prompt
- Setup your build environment:
```
export PYTHON=<full path to your python.exe program>
export OMNIORBBASE=<full path to your OmniORB installation folder>
```
- Follow the build instructions in the old README file
