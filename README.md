# CMPSCI-4760 P6

## Project Goal:

* designing and implementing a memory management module for our Operating System
Simulator oss.
* implementing the FIFO (clock) page replacement algorithms. 

## How to run
Commands to run project from hoare server:
```
$ make
$ ./oss
```
Output file will be osslog.

## Git

https://github.com/SanzidaH/CMPSCI4760-p6.git

## Task Summary

> OSS forks multiple children or user processes at random times checking total process in system is less than 100.
> OSS sets up pcb, frame table and page tables.
> User processes and OSS can communicate through Semaphore.
> User processes request a read/write of a memory address and OSS grants it.
> OSS  runs simple deadlock detection algortihm periodically (checking if all processes are waiting) and kill processes to resolve the issue. 
> Emplty grame table when full.



