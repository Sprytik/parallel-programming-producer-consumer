# parallel-programming-producer-consumer

## Overview

This project implements a parallel multithreaded process in C using POSIX threads
(pthreads) on Linux. It simulates producer-consumer problem with 
synchronization mechanisms to ensure safe concurrent access to shared resources.
The work is part of the "Parallel programming" course in KPI University.

Content:
- A producer thread that inserts data into a dynamic circular linked list.
- Three consumer threads that remove/process elements from the list.
- Two additional threads that operate on atomic variables and synchronize using a barrier.

The project demonstrates use of:
- `pthread_mutex_t` mutex 
- `pthread_cond_t` conditional variables 
- `sem_t semaphore` 
- `pthread_barrier_t` barrier 
- atomic instructions 
- dynamic memory allocation
- dynamic data structure


## Features

- Concurrent producer-consumer communication
- Safe access to a shared linked list structure
- Use of atomic variables
- Each thread operates in an infinite loop 


## Compilation and Usage

### Exit

Program exits via Ctrl+C interrupt signal

```bash
make
```

## Requirements

-Linux-based OS
-GCC compiler with pthread support
-POSIX-compliant terminal


