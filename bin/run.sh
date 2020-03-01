#!/bin/bash

# Compile the program with the Xlib flags
g++-8 -std=c++17 -O2 -Wall -o ./../File_Explorer ./../src/File_Explorer.cpp \
 ./../src/FWindow.cpp ./../src/FElement.cpp -L /usr/X11R6/lib \
  -lX11 -lm -lstdc++fs

./../File_Explorer