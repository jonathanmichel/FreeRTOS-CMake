# FreeRTOS-CMake

The goal of this repository is to try to replace Make used with FreeRTOS by CMake in order to integrate FreeRTOS into F' CMake compilation process.

This work is based on the [Posix GCC Demo](https://github.com/FreeRTOS/FreeRTOS/tree/master/FreeRTOS/Demo/Posix_GCC) of FreeRTOS. The goal is to use the [Posix/Linux Simulator Demo for FreeRTOS](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html).

## Prerequisites
- GNU Make 4.1
- cmake version 3.10.2
- gcc 7.5.0

Tested on Ubuntu 18.04 distribution Lts with Windows WSL

## Running example with Make
After cloning repository, ensure submodule (and subsubmodules) are correctly initialized and updated. 

Project [Makefile](Makefile) is based on the [demo one](https://github.com/FreeRTOS/FreeRTOS/blob/master/FreeRTOS/Demo/Posix_GCC/Makefile) but has been simplified to remove FreeRTOS+ dependencies. 

You can try make it with 

```
❯ make
```

And run it
```
❯ ./build/posix_demo
Starting echo blinky demo
Message received from task
Message received from software timer
Message received from task
Message received from task
Message received from software timer
Message received from task
```

## CMake usage

In order to replace Make by CMake a [CMakeLists.txt](CMakeLists.txt) file has been writen. Its goal is to include FreeRTOS and compile the demo code. You can try it with:

```
cmake .
```

At this step, a new Makefile has been generated (A copy of the initial is in [Makefile.back](Makefile.back)). Once again, you can make the program with:

```
❯ make
```
