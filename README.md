# MeMS: Memory Management System [CSE231 OS Assignment 3]
[Documentation](https://docs.google.com/document/d/1Gs9kC3187lLrinvK1SueTc8dHCJ0QP43eRlrCRlXiCY/edit?usp=sharing)
---

### How to run the example.c
After implementing functions in mems.h follow the below steps to run example.c file
```
$ make
$ ./example
```

## Table of Contents
1. [Problem Statement](#problem-statement)
2. [Prerequisites](#prerequisites)
3. [Code Structure](#code-structure)
    - [Header Files](#header-files)
    - [Data Structures](#data-structures)
    - [Global Variables](#global-variables)
4. [Code Walkthrough](#code-walkthrough)
5. [Output](#output)

## Problem Statment
- Implement a custom memory management system (MeMS) using the C programming language. MeMS should utilize the system calls mmap and munmap for memory allocation and deallocation, respectively

## Prerequisites
- C compiler (e.g., GCC)
- POSIX-compliant system (Linux, Unix, macOS): Parallels VM for this case

## Code Structure
### Header Files
```
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
```
### Data Structures
```
struct node {
// Structure members
};
```

### Global Variables
```
struct node *headPtr;
int currIndex, totalNodes, pageMultiple;
int sizeNode = sizeof(struct node);
#define PAGE_SIZE 4096
```

## Code Walkthrough

### mems_init()
```
void mems_init() {
    // Initialization code
}
```
### mems_malloc(size_t reqSize)
```
void *mems_malloc(size_t reqSize) {
    // Memory allocation code
}
```

### mems_free(void *vPtr)
```
void mems_free(void *vPtr) {
    // Memory deallocation code
}
```
### mems_print_stats()
```
void mems_print_stats() {
    // Print memory statistics
}
```
### mems_get(void *vPtr)
```
void *mems_get(void *vPtr) {
    // Get physical memory address
}
```
### mergeCo()
```
void mergeCo() {
    // Merge contiguous holes
}
```
# To Run
- locate to the folder named "2022357_2022293"
```agsl
$ make
$ ./example
```

# Output

```agsl
------- Allocated virtual addresses [mems_malloc] -------
Virtual address: 0
Virtual address: 1000
Virtual address: 2000
Virtual address: 3000
Virtual address: 4096
Virtual address: 5096
Virtual address: 6096
Virtual address: 7096
Virtual address: 8192
Virtual address: 9192

------ Assigning value to Virtual address [mems_get] -----
Virtual address: 0	 Physical Address: 281473053999104
Value written: 200

--------- Printing Stats [mems_print_stats] --------
----- MeMS SYSTEM STATS ----
MAIN[0:4095]-> P[0:999]<->P[1000:1999]<->P[2000:2999]<->P[3000:3999]<->P[4000:4095]
MAIN[4096:8191]-> P[4096:5095]<->P[5096:6095]<->P[6096:7095]<->P[7096:8095]<->P[8096:8191]
MAIN[8192:12287]-> P[8192:9191]<->P[9192:10191]<->P[10192:12287]-> NULL

Pages Used : 3
Space unused : 2288
Main Chain Length: 3
Sub-Chain Length Array: [5 ,5 ,3 ,]


--------- Freeing up the memory [mems_free] --------
----- MeMS SYSTEM STATS ----
MAIN[0:4095]-> P[0:999]<->P[1000:1999]<->P[2000:2999]<->P[3000:4095]
MAIN[4096:8191]-> P[4096:5095]<->P[5096:6095]<->P[6096:7095]<->P[7096:8095]<->P[8096:8191]
MAIN[8192:12287]-> P[8192:9191]<->P[9192:10191]<->P[10192:12287]-> NULL

Pages Used : 3
Space unused : 3288
Main Chain Length: 3
Sub-Chain Length Array: [4 ,5 ,3 ,]

----- MeMS SYSTEM STATS ----
MAIN[0:4095]-> P[0:999]<->P[1000:1999]<->P[2000:2999]<->P[3000:3999]<->P[4000:4095]
MAIN[4096:8191]-> P[4096:5095]<->P[5096:6095]<->P[6096:7095]<->P[7096:8095]<->P[8096:8191]
MAIN[8192:12287]-> P[8192:9191]<->P[9192:10191]<->P[10192:12287]-> NULL

Pages Used : 3
Space unused : 2288
Main Chain Length: 3
Sub-Chain Length Array: [5 ,5 ,3 ,]


--------- Unmapping all memory [mems_finish] --------

```


