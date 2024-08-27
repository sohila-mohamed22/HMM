# 📚 Heap Memory Manager (HMM)

This project implements a custom Heap Memory Manager (HMM) to replace the standard `libc` dynamic memory management functions. The goal is to manage dynamic memory using custom implementations that interface with a real heap managed through `sbrk()`.

## 📑 Table of Contents

1. [Overview](#overview)
2. [Project Structure](#project-structure)
   - [Source Files](#source-files)
3. [Usage](#usage)
   - [HmmAlloc](#hmmallocsize-t-size)
   - [HmmCalloc](#hmmcallocsize-t-num-size-t-size)
   - [HmmRealloc](#hmmreallocvoid--ptr-size-t-new-size)
   - [HmmFree](#hmmfreevoid--ptr)
4. [Compilation and Setup](#compilation-and-setup)
   - [Step 1: Compile the Shared Library](#step-2-compile-the-shared-library)
   - [Step 2: Preload the Custom HMM Library](#step-4-preload-the-custom-hmm-library)
5. [Flowcharts](#flowcharts)
   - [HmmAlloc Function](#hmmalloc-function)
   - [HmmFree Function](#hmmfree-function)

## 🛠️ Overview

This project replaces the standard memory management functions (`malloc`, `free`, `calloc`, and `realloc`) with custom implementations. It utilizes `sbrk()` to manage a dynamic heap segment, and includes wrapper functions in `heap.c` that call core memory management functions defined in `FreeList.c`.

### Wrapper Functions

In **`heap.c`**, the following wrapper functions are defined:
- **`malloc(size_t size)`**: Calls `HmmAlloc(size)`, which is implemented in the same file.
- **`free(void *ptr)`**: Calls `HmmFree(ptr)`, which is also implemented in `heap.c`.
- **`calloc(size_t num, size_t size)`**: Calls `HmmCalloc(num, size)`, implemented in `heap.c`.
- **`realloc(void *ptr, size_t size)`**: Calls `HmmRealloc(ptr, size)`, implemented in `heap.c`.

These functions manage memory allocation, deallocation, and resizing, interfacing with a dynamically managed heap.

## 📁 Project Structure

### Source Files

- **`heap.c`**: Contains both the wrapper functions and core memory management functions:
  - **`malloc(size_t size)`**: Delegates to `HmmAlloc(size)`.
  - **`free(void *ptr)`**: Delegates to `HmmFree(ptr)`.
  - **`calloc(size_t num, size_t size)`**: Delegates to `HmmCalloc(num, size)`.
  - **`realloc(void *ptr, size_t size)`**: Delegates to `HmmRealloc(ptr, size)`.

- **`FreeList.c`**: Implements functions for managing a free list:
  - **`uint8_t calculate_decreases_in_program_break()`**: Iterate through the free list to find contiguous memory blocks.
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

### ✨`void *HmmCalloc(size_t num, size_t size)`

Allocates memory for an array of `num` elements, each of `size` bytes, and initializes all bytes to zero.

- **Parameters**

  - **`num`**: The number of elements to allocate.
      - **Type**: `size_t`
      - **Description**: Specifies how many elements you want to allocate.

  - **`size`**: The size of each element in bytes.
     - **Type**: `size_t`
     - **Description**: Specifies the size of each element in bytes.

- **Returns**
   - **Pointer to the allocated memory block**: If successful, returns a pointer to the allocated memory block.
   - **`NULL`**: If allocation fails, returns `NULL`.

### `void *HmmRealloc(void *ptr, size_t new_size)`

Resizes an existing memory block to the new size. The content of the old block is copied to the new block if the size increases.

- **Parameters**
   - **`ptr`**: The pointer to the existing memory block that needs to be resized.
       - **Type**: `void *`
       - **Description**: The pointer to the currently allocated memory block.

    - **`new_size`**: The new size of the memory block in bytes.
         - **Type**: `size_t`
         - **Description**: Specifies the new size of the memory block.

- **Returns**
     - **Pointer to the resized memory block**: If successful, returns a pointer to the resized memory block.
     - **`NULL`**: If reallocation fails, returns `NULL`. The original memory block remains unchanged.
 
### `void HmmFree(void *ptr)`

Frees a previously allocated memory block.

- **Parameters**

    - **`ptr`**: The pointer to the memory block that needs to be freed.
        - **Type**: `void *`
        - **Description**: The pointer to the memory block that you want to deallocate.
 
    - **Returns**
       - **None**: This function does not return a value.

## ⚙️ Compilation and Setup

### Step 1: Compile the Shared Library
- gcc -fPIC -shared -o lib/libhmm.so src/heap.c src/FreeList.c

### Step 2: Preload the Custom HMM Library
- **Example**: Preload HMM library with ls command
  - **LD_PRELOAD=./lib/libhmm.so ls**

- **Example**: Preload HMM library with vim editor
  - **LD_PRELOAD=./lib/libhmm.so vim**

- **Example**: Preload HMM library with bash shell
  - **LD_PRELOAD=./lib/libhmm.so bash**
 
## 🗺️ Flowcharts
- **HmmAlloc Function**
The following flowchart illustrates the logic of the HmmAlloc function:
  ![Flowcharts](https://github.com/user-attachments/assets/2051910b-4918-47cc-9f31-20ec66b685de)

- **HmmFree Function**
The following flowchart illustrates the logic of the HmmFree function:
 ![Flowcharts - Page 1 (1)](https://github.com/user-attachments/assets/dd2b4df6-4164-411f-ac78-4063a786db65)
