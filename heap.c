#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "heap.h"
#include "FreeList.h"

#define SIZE (1024 * 1024 * 1024) /* 1 GB - Total memory size */
#define PAGE_SIZE (200 * 1024)    /* 200 KB - Size of each memory page */
#define FREE_SIZE (128 * 1024)    /* 128 KB - Size of the free block threshold */

// Simulated program break for memory management
char *programBreak;

/* Size of a word in bytes */
size_t word_size = 8; /* 8 bytes */

/**
 * Custom implementation of malloc to allocate memory.
 * This function uses the HmmAlloc function to handle memory allocation.
 *
 * @param size The size of memory to allocate in bytes.
 * @return A pointer to the allocated memory, or NULL if allocation fails.
 */
void *malloc(size_t size)
{
    return HmmAlloc(size);
}

/**
 * Custom implementation of free to deallocate memory.
 * This function uses the HmmFree function to handle memory deallocation.
 *
 * @param ptr A pointer to the memory to be freed. If NULL, the function does nothing.
 */
void free(void *ptr)
{
    if (ptr != NULL)
    {
        HmmFree(ptr);
    }
}

/**
 * Custom implementation of calloc to allocate and zero-initialize memory.
 * This function uses the HmmCalloc function to handle memory allocation and initialization.
 *
 * @param nmemb The number of elements to allocate.
 * @param size The size of each element in bytes.
 * @return A pointer to the allocated and zero-initialized memory, or NULL if allocation fails.
 */
void *calloc(size_t nmemb, size_t size)
{
    return HmmCalloc(nmemb, size);
}

/**
 * Custom implementation of realloc to resize a previously allocated memory block.
 * This function uses the HmmRealloc function to handle resizing of the memory block.
 *
 * @param ptr A pointer to the previously allocated memory block.
 * @param size The new size of the memory block in bytes.
 * @return A pointer to the resized memory block, or NULL if resizing fails.
 */
void *realloc(void *ptr, size_t size)
{
    // If the pointer is NULL, allocate new memory
    if (ptr == NULL)
    {
        return malloc(size);
    }

    // Otherwise, resize the existing memory block
    return HmmRealloc(ptr, size);
}

/**
 * @brief Allocates memory of the requested size and manages the program break if necessary.
 *
 * This function allocates a block of memory of at least the requested size. If the requested size is
 * smaller than the minimum block size, it is adjusted. The function also handles memory allocation
 * by adjusting the program break and searching for the best fit in the freelist.
 *
 * @param requestedSize The size of the memory block to allocate.
 * @return Pointer to the allocated memory block, or NULL if allocation fails.
 */
void *HmmAlloc(size_t requestedSize)
{
    void *allocatedAddress = NULL;     // Pointer to the allocated memory block
    char *previousProgramBreak = NULL; // Temporary pointer for program break management
    char *initialProgramBreak;         // Pointer to track the initial program break position
    static uint8_t initializationFlag = 0; // Flag to check if the program break has been initialized
  
    // Initialize the program break if it hasn't been initialized yet
    if (initializationFlag == 0)
    {
        initialProgramBreak = increase_program_break(0);
        programBreak = initialProgramBreak;
        initializationFlag++;
    }

    // Adjust the requested size to the minimum block size if it's too small
    if (requestedSize < 24)
    {
        requestedSize = 24;
    }

    // Align the requested size to the nearest multiple of the word size
    requestedSize = (requestedSize + word_size - 1) & ~(word_size - 1);

    // Check if the program break is pointing to the initial position
    if (programBreak == initialProgramBreak)
    {
        previousProgramBreak = programBreak;
        programBreak = (char *)increase_program_break(PAGE_SIZE);
        
        // Check if sbrk increment was successful
        if (programBreak == NULL)
        {
            allocatedAddress = NULL;
            programBreak = previousProgramBreak;
        }
        else
        {
            *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
            insert_block_into_freelist(previousProgramBreak + 24);
            
            // Find a suitable block in the freelist
            allocatedAddress = find_best_fit_block(requestedSize);
 	    while((allocatedAddress-24) == NULL)
            {
                // Allocate additional memory if no suitable block was found
                previousProgramBreak = programBreak;
                programBreak = (char *)increase_program_break(PAGE_SIZE);
                
                if (programBreak == NULL)
                {
                    allocatedAddress = NULL;
                    programBreak = previousProgramBreak;
                }
                else
                {
                    *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
                    insert_block_into_freelist(previousProgramBreak + 24);
                    allocatedAddress = find_best_fit_block(requestedSize);
                }
            }
        }
    }
    else
    {
        // Search for a suitable block in the freelist
        allocatedAddress = find_best_fit_block(requestedSize);
 	while((allocatedAddress-24) == NULL)
        {
            // Allocate additional memory if no suitable block was found
            previousProgramBreak = programBreak;
            programBreak = (char *)increase_program_break(PAGE_SIZE);
            
            if (programBreak == NULL)
            {
                allocatedAddress = NULL;
                programBreak = previousProgramBreak;
            }
            else
            {
                *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
                insert_block_into_freelist(previousProgramBreak + 24);
                allocatedAddress = find_best_fit_block(requestedSize);
            }
        }
    }

    return allocatedAddress;
}


/**
 * @brief Frees a previously allocated memory block and adjusts the program break if possible.
 *
 * This function deallocates the memory block pointed to by `ptr`, adds it to the freelist, and tries to reduce
 * the program break if sufficient free memory is available. The program break is adjusted based on the amount
 * of contiguous free memory identified.
 *
 * @param ptr Pointer to the memory block to be freed.
 */
void HmmFree(void *blockPtr)
{
    char *newProgramBreak = NULL;  // Temporary pointer for program break adjustment
    uint32_t reductionCount = 0;      // Number of program break decrements

    /* Add the freed memory block to the freelist */
    insert_block_into_freelist(blockPtr);

    /* Determine how many times the program break can be decreased based on free memory */
    reductionCount = calculate_decreases_in_program_break();

    /* If the program break can be decreased, perform the adjustment */
    if (reductionCount > 0)
    {
        newProgramBreak = (char *)decrease_program_break(reductionCount * FREE_SIZE);
    }

    /* Update the program break if the decrement was successful */
    if (newProgramBreak != NULL)
    {
        programBreak = newProgramBreak;
    }
}

/**
 * @brief Allocates memory for an array of elements, initializing all bytes to zero.
 *
 * This function allocates a block of memory for an array of `nmemb` elements, each of size `size`, and initializes
 * the entire block to zero. If the allocation fails, it returns NULL.
 *
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element.
 * @return void* Pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *HmmCalloc(size_t nmemb, size_t size)
{
    void *memory_block = NULL;
    uint64_t total_size = nmemb * size;

    /* Allocate memory for the specified number of elements */
    memory_block = HmmAlloc(total_size);

    /* If allocation succeeded, initialize the memory to zero */
    if (memory_block != NULL)
    {
        memory_block = memset(memory_block, 0, total_size);
    }

    return memory_block;
}

/**
 * @brief Reallocates a memory block to a new size, either expanding or shrinking it.
 *
 * This function adjusts the size of a previously allocated memory block. If the requested size is
 * larger than the current size, it attempts to find a suitable block or extend the memory. If the
 * requested size is smaller, it reduces the size of the allocated block as needed.
 *
 * @param originalPtr Pointer to the previously allocated memory block.
 * @param newSize The desired new size for the memory block.
 * @return Pointer to the reallocated memory block, or NULL if allocation fails.
 */
void *HmmRealloc(void *originalPtr, size_t newSize)
{
    char *previousProgramBreak = NULL; // Temporary pointer for program break management
    uint64_t currentBlockSize;         // Size of the current memory block
    uint64_t freeSpaceSize;            // Size of the free space after resizing
    void *newBlockPtr = NULL;          // Pointer to the new memory block

    // Handle case where the new size is zero (free the memory block)
    if (newSize == 0)
    {
         newBlockPtr = malloc(24);
    }
    
    else
    {
	    // Calculate the current block size
            currentBlockSize = *(uint64_t *)(originalPtr - 24);

	    // Handle case where the new size is larger than the current size
            if (newSize > currentBlockSize)
	    {
	    	    // Try to find a suitable free block that can accommodate the new size
                    newBlockPtr = find_best_fit_block(newSize - currentBlockSize);

	    	    if ((newBlockPtr-24) == NULL)
	    	    {
			    // If no suitable block is found, allocate additional memory
                            previousProgramBreak = programBreak;
			    programBreak = (char *)increase_program_break(PAGE_SIZE);
            
			    // Check if sbrk increment was successful
			    if (programBreak == NULL)
			    {
		    		    newBlockPtr = NULL;
		    		    programBreak = previousProgramBreak;
			    }
			    else
			    {
		    		    *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
		    		    insert_block_into_freelist(previousProgramBreak + 24);
                
		    		    // Try to find a suitable block again
                                    newBlockPtr = find_best_fit_block(newSize);
		    		    while ((newBlockPtr-24) == NULL)
		    		    {
					    previousProgramBreak = programBreak;
					    programBreak = (char *)increase_program_break(PAGE_SIZE);
                    
					    if (programBreak == NULL)
					    {
			    			    newBlockPtr = NULL;
			    			    programBreak = previousProgramBreak;
					    }
					    else
					    {
			    			    *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
			    			    insert_block_into_freelist(previousProgramBreak + 24);
			    			    newBlockPtr = find_best_fit_block(newSize);
					    }
		    		    }
		    		    // Copy the contents from the old block to the new block
                                    newBlockPtr = memcpy(newBlockPtr, originalPtr, currentBlockSize);
		    		    HmmFree(originalPtr);
			    }
	    	    }
	    	    else if ((originalPtr + 24 + currentBlockSize) == newBlockPtr)
	    	    {
			    // Extend the current block if possible
                            *(uint64_t *)(originalPtr - 24) = newSize + 24;
			    newBlockPtr = originalPtr;
	    	    }
	    	    else
	    	    {
			    // Free the newly found block and allocate additional memory
                            HmmFree(newBlockPtr);
			    previousProgramBreak = programBreak;
			    programBreak = (char *)increase_program_break(PAGE_SIZE);
            
			    if (programBreak == NULL)
			    {
		    		    newBlockPtr = NULL;
		    		    programBreak = previousProgramBreak;
			    }
			    else
			    {
		    		    *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
		    		    insert_block_into_freelist(previousProgramBreak + 24);
		    		    newBlockPtr = find_best_fit_block(newSize);
		    		    while ((newBlockPtr-24) == NULL)
		    		    {
					    previousProgramBreak = programBreak;
					    programBreak = (char *)increase_program_break(PAGE_SIZE); 
					    if (programBreak == NULL)
					    {
			    			    newBlockPtr = NULL;
			    			    programBreak = previousProgramBreak;
					    }
					    else
					    {
			    			    *(uint64_t *)previousProgramBreak = (uint64_t)(programBreak - previousProgramBreak) - 24;
			    			    insert_block_into_freelist(previousProgramBreak + 24);
			    			    newBlockPtr = find_best_fit_block(newSize);
					    }
		    		    }
		    		    newBlockPtr = memcpy(newBlockPtr, originalPtr, currentBlockSize);
		    		    HmmFree(originalPtr);
			    }
	    	    }
	    }
	    else
	    {
	    	    // Handle case where the new size is smaller than or equal to the current size
                    if (newSize < 24)
	    	    {
			    newSize = 24;
	    	    }
        
	    	    // Align the new size to the nearest multiple of the word size
		    newSize = (newSize + word_size - 1) & ~(word_size - 1);

	    	    freeSpaceSize = currentBlockSize - newSize;
	    	    if (freeSpaceSize <= 24)
	    	    {
			    newBlockPtr = originalPtr;
	    	    }
	    	    else
	    	    {
			    // Adjust the free space size to be a multiple of the word size
                            freeSpaceSize = (freeSpaceSize + word_size - 1) & ~(word_size - 1);
			    *(uint64_t *)(originalPtr - 24) = newSize;
			    *(uint64_t *)(originalPtr + newSize) = freeSpaceSize - 24;
			    HmmFree(originalPtr + newSize + 24);
			    newBlockPtr = originalPtr;
	    	    }
	    }
    }
    return newBlockPtr;
}

/**
 * @brief Increases the program break by a specified increment.
 *
 * This function increases the program's data space by the specified increment using the `sbrk` system call.
 * It returns the new program break address after the increment. If the memory allocation fails, it returns `NULL`.
 *
 * @param increment The number of bytes to increase the program break by.
 * @return void* The new program break address, or `NULL` if the allocation fails.
 */
void *increase_program_break(size_t increment)
{
    void *current_break = sbrk(increment); // Attempt to increase the program break

    // Check if the sbrk call was successful
    if (current_break == (void *)-1) {
        return NULL; // Return NULL if the memory allocation failed
    }

    // Get the new program break after the increment
    current_break = sbrk(0);
    return current_break;
}

/**
 * @brief Decreases the program break by a specified decrement.
 *
 * This function decreases the program's data space by the specified decrement using the `sbrk` system call.
 * It returns the new program break address after the decrement. If the memory deallocation fails, it returns `NULL`.
 *
 * @param decrement The number of bytes to decrease the program break by.
 * @return void* The new program break address, or `NULL` if the deallocation fails.
 */
void *decrease_program_break(size_t decrement)
{
    void *current_break = sbrk(-decrement); // Attempt to decrease the program break

    // Check if the sbrk call was successful
    if (current_break == (void *)-1) {
        return NULL; // Return NULL if the memory deallocation failed
    }

    // Get the new program break after the decrement
    current_break = sbrk(0);
    return current_break;
}

