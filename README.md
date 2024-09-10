# 📚 Heap Memory Manager (HMM)

This project implements a custom Heap Memory Manager (HMM) to replace the standard `libc` dynamic memory management functions. The goal is to manage dynamic memory using custom implementations that interface with a real heap managed through `sbrk()`.

## 📑 Table of Contents

1. [Overview](#Overview)
2. [Project Structure](#Project-Structure)
   - [Source Files](#Source-Files)
3. [Usage](#Usage)
   - [HmmAlloc](#HmmAllocsize-t-size)
   - [HmmCalloc](#HmmCallocsize-t-num-size-t-size)
   - [HmmRealloc](#HmmReallocvoid--ptr-size-t-new-size)
   - [HmmFree](#HmmFreevoid--ptr)
   - [Example](#Example)
4. [Flowcharts](#Flowcharts)
   - [HmmAlloc Function](#HmmAlloc-Function)
   - [HmmCalloc Function](#HmmCalloc-Function)
   - [HmmRealloc Function](#HmmRealloc-Function)
   - [HmmFree Function](#HmmFree-Function)
5. [Installation and Compilation](#Installation-and-Compilation)
   - [Step 1:Clone the Repository](#step-1-Clone-the-Repository)
   - [Step 2: Compile the Shared Library](#step-2-Compile-the-Shared-Library)
   - [Step 3: Preload the Custom HMM Library](#step-3-Preload-the-Custom-HMM-Library)
## 🛠️ Overview

### What is HMM?

**Heap Memory Manager (HMM)** is a custom implementation of a memory management system designed to handle dynamic memory allocation in a more controlled way than the standard `malloc`, `free`, `calloc`, and `realloc` functions provided by the C standard library. HMM uses the `sbrk()` system call to manage a heap segment, allowing it to allocate, resize, and deallocate memory blocks.

### Role of HMM

The primary role of HMM is to manage memory dynamically with the following capabilities:
- **Allocate Memory**: Provide memory blocks of requested sizes.
- **Resize Memory**: Adjust the size of existing memory blocks while preserving their contents.
- **Deallocate Memory**: Free previously allocated memory blocks to be reused.
- **Maintain Free List**: Keep track of free memory blocks to efficiently allocate new memory.

## 📁 Project Structure

### Source Files

- **`heap.c`**: Contains both the wrapper functions and core memory management functions:
  - **`malloc(size_t size)`**: Delegates to `HmmAlloc(size)`.
  - **`free(void *ptr)`**: Delegates to `HmmFree(ptr)`.
  - **`calloc(size_t num, size_t size)`**: Delegates to `HmmCalloc(num, size)`.
  - **`realloc(void *ptr, size_t size)`**: Delegates to `HmmRealloc(ptr, size)`.

- **`FreeList.c`**: Implements functions for managing a free list:
  - **`uint8_t calculate_decreases_in_program_break()`**: Iterates through the free list to find last contiguous memory blocks.
  - **`void insert_block_into_freelist(void *blockPtr)`**: Inserts a block into the free list.
  - **`void insert_node_at_start(void *ptr)`**: Inserts a node at the start of the free list.
  - **`void append_to_freelist_end(void *blockPtr)`**: Appends a node to the end of the free list.
  - **`void insert_node_between(FreeListNode *currentNodePtr, void *blockPtr)`**: Inserts a node between two existing nodes in the free list.
  - **`void remove_freelist_node(void *nodePtr)`**: Removes a node from the free list.
  - **`void *find_best_fit_block(uint64_t requestedSize)`**: Finds the best fit block in the free list for the requested size.

## 🛠️ Usage

### `void *HmmAlloc(size_t size)`

Allocates a block of memory of the specified size.

- **Parameters**:
  - `size`: Size of the memory block in bytes.
- **Returns**:
  - Pointer to the allocated memory block, or `NULL` if allocation fails.

### `void *HmmCalloc(size_t num, size_t size)`

Allocates memory for an array of `num` elements, each of `size` bytes, and initializes all bytes to zero.

- **Parameters**:
  - **`num`**: The number of elements to allocate.
  - **`size`**: The size of each element in bytes.
- **Returns**:
  - Pointer to the allocated memory block if successful, or `NULL` if allocation fails.

### `void *HmmRealloc(void *ptr, size_t new_size)`

Resizes an existing memory block to the new size. The content of the old block is copied to the new block if the size increases.

- **Parameters**:
  - **`ptr`**: The pointer to the existing memory block that needs to be resized.
  - **`new_size`**: The new size of the memory block in bytes.
- **Returns**:
  - Pointer to the resized memory block if successful, or `NULL` if reallocation fails. The original memory block remains unchanged.

### `void HmmFree(void *ptr)`

Frees a previously allocated memory block.

- **Parameters**:
  - **`ptr`**: The pointer to the memory block that needs to be freed.
- **Returns**:
  - None. This function does not return a value.

### Example

Here is an example demonstrating how to use all the HMM functions together:

```c
#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
#include "FreeList.h"

int main() {
    // Allocate memory for an integer
    int *num = (int *)malloc(sizeof(int));
    if (num == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    *num = 42;
    printf("Allocated integer value: %d\n", *num);

    // Reallocate memory to expand it to an array of integers
    int *array = (int *)realloc(num, 10 * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "realloc failed\n");
        return 1;
    }

    // Initialize the array
    for (size_t i = 0; i < 10; i++) {
        array[i] = i * 10;
    }
    printf("Reallocated array values:\n");
    for (size_t i = 0; i < 10; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    // Allocate memory for a zero-initialized array
    int *zero_array = (int *)calloc(5, sizeof(int));
    if (zero_array == NULL) {
        fprintf(stderr, "calloc failed\n");
        return 1;
    }
    printf("Calloc zero-initialized array values:\n");
    for (size_t i = 0; i < 5; i++) {
        printf("%d ", zero_array[i]);
    }
    printf("\n");

    // Free the allocated memory
    free(array);
    free(zero_array);

    return 0;
}
```

## 🗺️ Flowcharts
- **HmmAlloc Function**
The following flowchart illustrates the logic of the HmmAlloc function:
![Flowcharts - Page 2 (1)](https://github.com/user-attachments/assets/538120ff-125d-4072-acf6-813dca412a73)

- **HmmCalloc Function**
The following flowchart illustrates the logic of the HmmCalloc function:
![Flowcharts - Page 4](https://github.com/user-attachments/assets/f3c901ec-5412-4e1e-9bdb-c143423208f6)

- **HmmRealloc Function**
The following flowchart illustrates the logic of the HmmRealloc function:
![Flowcharts - Page 4 (1)](https://github.com/user-attachments/assets/9f385904-5f2b-48d0-9269-a7bacba768be)

- **HmmFree Function**
The following flowchart illustrates the logic of the HmmFree function:
![Flowcharts - Page 3 (1)](https://github.com/user-attachments/assets/883668d0-c0bc-4594-b215-f7f0934992b3)


## ⚙️ Installation and Compilation 

### Step 1: Clone the Repository
```bash
git clone https://github.com/your-username/your-repo.git
cd your-repo
```

### Step 2: Compile the Shared Library
```bash
gcc -fPIC -shared -o lib/libhmm.so src/heap.c src/FreeList.c
```

### Step 3: Preload the Custom HMM Library
- **Example**: Preload HMM library with ls command
```bash
LD_PRELOAD=./lib/libhmm.so ls
```
- **Example**: Preload HMM library with vim editor
```bash
LD_PRELOAD=./lib/libhmm.so vim
```
- **Example**: Preload HMM library with bash shell
```bash
LD_PRELOAD=./lib/libhmm.so bash
 ```

