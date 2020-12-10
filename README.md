# FreeRTOS-CMake

This repository covers the replacement of Make used with FreeRTOS by CMake.

The goal is to use the [Posix/Linux Simulator Demo for FreeRTOS](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html) in order to run FreeRTOS on Linux. This work is based on the [Posix GCC Demo](https://github.com/FreeRTOS/FreeRTOS/tree/master/FreeRTOS/Demo/Posix_GCC) of FreeRTOS. 

## Prerequisites
- GNU Make 4.1
- CMake version 3.10.2
- GCC 7.5.0

Tested on Ubuntu 18.04 Lts distribution with Windows WSL.

## Project structure
Project is a simple demo responsible to create two tasks communicating with queue and triggered by timers. It will print received messages in the console.
- [main.c](main.c) contains application code
- [console.h/c](console.h) are used to trace code by avoiding concurrency issues
- [FreeRTOSConfig.h](FreeRTOSConfig.h) is need by FreeRTOS. It contains application specific definitions that are adjusted for a particular hardware and application requirements. This one comes from the [POSIX demo](https://github.com/FreeRTOS/FreeRTOS/blob/master/FreeRTOS/Demo/Posix_GCC/FreeRTOSConfig.h).
- [Makefile](Makefile) used for direct compilation with Make
- [CMakeLists.txt](CMakeLists.txt) used by CMake
- [Makefile.back](Makefile.back) Backup of the manual [Makefile](Makefile)

## Running example with Make
After cloning repository, ensure submodule (and sub-submodules) are correctly initialized and updated. 

Project [Makefile](Makefile) is based on the [FreeRTOS demo one](https://github.com/FreeRTOS/FreeRTOS/blob/master/FreeRTOS/Demo/Posix_GCC/Makefile) but has been simplified to remove FreeRTOS+ dependencies. 

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

At this step, a new Makefile has been generated (a copy of the initial one is in [Makefile.back](Makefile.back)). Once again, you can make the program with:

```
❯ make
```

And run it with:
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

