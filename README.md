# 📚 Heap Memory Manager (HMM)

This project implements a custom Heap Memory Manager (HMM) to replace the standard `libc` dynamic memory management functions. The goal is to manage dynamic memory using custom implementations that interface with a real heap managed through `sbrk()`.

## 📑 Table of Contents

1. [Overview](#overview)
2. [Project Structure](#project-structure)
   - [Source Files](#source-files)
3. [Compilation and Setup](#compilation-and-setup)
   - [Step 1: Environment Preparation](#step-1-environment-preparation)
   - [Step 2: Compile the Shared Library](#step-2-compile-the-shared-library)
   - [Step 3: Compile the Test Program](#step-3-compile-the-test-program)
4. [Usage Instructions](#usage-instructions)
   - [Step 4: Preload the Custom HMM Library](#step-4-preload-the-custom-hmm-library)
   - [Step 5: Run the Test Program](#step-5-run-the-test-program)
5. [Flowcharts](#flowcharts)
   - [HmmAlloc Function](#hmmalloc-function)
   - [HmmFree Function](#hmmfree-function)
6. [Testing and Validation](#testing-and-validation)
7. [Security Considerations](#security-considerations)
8. [Contribution Guidelines](#contribution-guidelines)
9. [License](#license)

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
 ├── src │ ├── heap.c # Wrapper functions and core HMM functions (HmmAlloc, HmmFree, etc.) │ └── FreeList.c # Optional: additional core implementation (if needed) ├── test │ └── test_program.c # Test program to validate the custom HMM library ├── lib # Directory for the compiled shared library ├── bin # Directory for the compiled test program └── README.md # Project documentation
