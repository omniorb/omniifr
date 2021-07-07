# OmniIFR2

Interface Repository for OmniORB, a CORBA ORB for C++ and Python.

Old official repository for previous versions up to 2.pre.1:
https://sourceforge.net/projects/omniifr/

See the old README for more information about the previous versions.

## How to build on Windows with Visual C++

Tested with:
- Visual 2005 SP6 (C++ version 8)
- Version 2.pre.1: Python 1.5.2 & OmniORB 4.1.7
- Version 2.pre.2: Python 3.5.3 & OmniORB 4.2.3

Build steps:
- Open a command prompt within the Visual build environment
- Set the ORB_INC and ORB_LIB variables to your OmniORB local installation
- Go to the `visual` subfolder
- Run the `build.bat` script

OmniIFR.exe is built in release mode with debug information in the `visual\build` subfolder.

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
