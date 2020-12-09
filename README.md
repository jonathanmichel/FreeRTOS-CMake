# FreeRTOS-CMake

This repository covers the replacement of Make used with FreeRTOS by CMake in order to integrate FreeRTOS into [FPrime](https://github.com/nasa/fprime) CMake compilation process.

The goal is to use the [Posix/Linux Simulator Demo for FreeRTOS](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html) in order to run FreeRTOS on Linux. This work is based on the [Posix GCC Demo](https://github.com/FreeRTOS/FreeRTOS/tree/master/FreeRTOS/Demo/Posix_GCC) of FreeRTOS. 

## Prerequisites
- GNU Make 4.1
- CMake version 3.10.2
- GCC 7.5.0

Tested on Ubuntu 18.04 Lts distribution with Windows WSL.

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

The compilation will fail with `fatal error: secure_context.h: No such file or directory` in [FreeRTOS/Source/portable/ARMv8M/non_secure/port.c:44](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/47338393f1f79558f6144213409f09f81d7c4837/portable/ARMv8M/non_secure/port.c#L44). 

By checking FreeRTOS sources, we can find mutliple occurences for this specific file in :
```
FreeRTOS\Source\portable\
├── ARMv8M\secure\context\secure_context.h
├── GCC\ARM_CM23\secure\secure_context.h
├── GCC\ARM_CM33\secure\secure_context.h
├── IAR\ARM_CM23\secure\secure_context.h
└── IAR\ARM_CM33\secure\secure_context.h
```

We could add a directory to include in CMakeLists.txt ([here](https://github.com/jonathanmichel/FreeRTOS-CMake/blob/167a5ac0153504d88c455072fb08df05f5c3fadd/CMakeLists.txt#L17)) by choosing a specific compiler and platform and re-run CMake. But it will result in other inclusion errors. 

It means that current CMakeLists.txt don't do the exact same task as inital Makefile. Why ? :(

