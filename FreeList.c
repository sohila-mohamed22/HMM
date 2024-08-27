#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeList.h"

// Pointer to the head of the free list. Initialized to NULL, indicating that the free list is currently empty.
FreeListNode *FreeListHead = NULL;

/**
 * @brief Calculates the number of times the program break can be decreased based on the remaining free memory.
 *
 * This function iterates through the free list and calculates the total free size of contiguous memory blocks.
 * If the total free size exceeds 128 KB, the function determines how many times the program break (memory allocated to the heap)
 * can be reduced by 128 KB chunks. It also removes these blocks from the free list and adjusts the remaining free memory
 * accordingly.
 *
 * @return uint8_t The number of times the program break can be decreased.
 */
uint8_t calculate_decreases_in_program_break()
{
    uint32_t decrease_count = 0;          // Tracks the number of 128 KB chunks that can be freed
    uint32_t contiguous_block_count = 0;  // Counts contiguous blocks found in the free list
    uint64_t total_free_size = 0;        // Accumulates the total size of contiguous free blocks
    FreeListNode *contiguous_blocks[10000] = {0};  // Array to store pointers to contiguous blocks
    FreeListNode *current_node = FreeListHead;     // Start at the head of the free list

    // Iterate through the free list to find contiguous memory blocks
    while (current_node != NULL)
    {
        if (contiguous_block_count == 0)
        {
            // Start a new contiguous block sequence
            total_free_size += current_node->length;
            contiguous_blocks[contiguous_block_count++] = current_node;
        }
        else
        {
            // Check if the current block is contiguous with the previous one
            if (current_node == (void *)(current_node->prev) + ((current_node->prev->length) + sizeof(FreeListNode)))
            {
                // If contiguous, add the block size and include it in the sequence
                total_free_size += (current_node->length + sizeof(FreeListNode));
                contiguous_blocks[contiguous_block_count++] = current_node;
            }
            else
            {
                // If not contiguous, reset the sequence and start a new one
                contiguous_block_count = 0;
                total_free_size = 0;
                total_free_size += current_node->length;
                contiguous_blocks[contiguous_block_count++] = current_node;
            }
        }
        current_node = current_node->next;
    }

    // Check if the total contiguous free size exceeds 128 KB
    if (total_free_size > (128 * 1024))
    {
        decrease_count++;  // Initial decrement for the first 128 KB
        contiguous_blocks[contiguous_block_count] = NULL;
        uint32_t i = 0;

        // Remove the contiguous blocks from the free list
        while (contiguous_blocks[i] != NULL)
        {
            remove_freelist_node(contiguous_blocks[i]);
            i++;
        }

        // Calculate additional decreases in program break for every extra 128 KB
        uint8_t remaining_size = (total_free_size - (128 * 1024));
        while (remaining_size > (128 * 1024))
        {
            decrease_count++;
            remaining_size -= (128 * 1024);
        }

        // If there is any remaining memory, reinsert it into the free list
        if (remaining_size > sizeof(FreeListNode))
        {
            FreeListNode *new_node = contiguous_blocks[0];
            *(uint64_t *)new_node = remaining_size;
            insert_block_into_freelist(((void *)new_node) + sizeof(FreeListNode));
        }
    }

    return decrease_count;
}

void insert_block_into_freelist(void *blockPtr)
{
   // If the freelist is empty or the new block should be placed at the start
    if (FreeListHead == NULL || (blockPtr - sizeof(FreeListNode)) < (void *)FreeListHead)
    {
        insert_node_at_start(blockPtr);
    }
    // If there is only one node in the freelist
    else if (FreeListHead->next == NULL)
    {
        append_to_freelist_end(blockPtr);
    }
    else
    {
        FreeListNode *currentNode = FreeListHead->next;

        // Traverse the freelist to find the correct position to insert the block
        while (currentNode != NULL)
        {
            if ((blockPtr - sizeof(FreeListNode)) < (void *)currentNode)
            {
                insert_node_between(currentNode, blockPtr);
                break;
            }
            currentNode = currentNode->next;
        }

        // If the correct position wasn't found, insert the block at the end
        if (currentNode == NULL)
        {
            append_to_freelist_end(blockPtr);
        }
    }
}

void insert_node_at_start(void *blockPtr)
{
     // Adjust the pointer to point to the new node, considering the node's size
    FreeListNode *newNode = (FreeListNode *)(blockPtr - sizeof(FreeListNode));
    
    // Check if the freelist is empty
    if (FreeListHead == NULL)
    {       
        // Set the new node as the head of the freelist
        FreeListHead = newNode;
        FreeListHead->length = newNode->length; 
        FreeListHead->prev = NULL;
	FreeListHead->next = NULL;
    }
    else
    {
        // Temporarily store the current head of the freelist
        FreeListNode *oldHead = FreeListHead;
    
	// Set the new head of the freelist to the block being inserted
        FreeListHead = newNode ;
     
    	// Initialize the new head's length field
        FreeListHead->length = newNode->length; 
    
	// The new head has no previous node, so set prev to NULL
	FreeListHead->prev = NULL;
    
        // Link the new head to the old head
        FreeListHead->next = oldHead;

        // Update previous pointer to the new head
        oldHead->prev = FreeListHead;
    }
}
void append_to_freelist_end(void *blockPtr)
{ 
    // Create a pointer to the new node, adjusting for the node's size
    FreeListNode *newNode = (FreeListNode *)(blockPtr - sizeof(FreeListNode));

    // Check if the freelist contains only one node
    if (FreeListHead->next == NULL)
    {
        // The new node's previous pointer should point to the current head of the freelist
        newNode->prev = FreeListHead;

        // Since this will be the last node, set its next pointer to NULL
        newNode->next = NULL;

        // Update the current head's next pointer to link to the new node
        FreeListHead->next = newNode;
    }
    else
    {
        // Traverse the list to find the last node
        FreeListNode *currentNode = FreeListHead->next;
        while (currentNode->next != NULL)
        {
            currentNode = currentNode->next;
        }

        // Set the new node's previous pointer to the last node
        newNode->prev = currentNode;

        // Set the new node's next pointer to NULL as it will be the last node
        newNode->next = NULL;

        // Update the last node's next pointer to point to the new node
        currentNode->next = newNode;
    }
}

void insert_node_between(FreeListNode *currentNodePtr, void *blockPtr)
{
    // Save the current node to a temporary variable
    FreeListNode *targetNode = currentNodePtr;

    // Create a new node at the specified block pointer, adjusting for the node's size
    FreeListNode *newNode = (FreeListNode *)(blockPtr - sizeof(FreeListNode));

    // Update the new node's pointers to link between the previous and current nodes
    newNode->prev = targetNode->prev;
    newNode->next = targetNode;

    // Update the previous node's next pointer to point to the new node
    targetNode->prev->next = newNode;

    // Update the target node's previous pointer to point to the new node
    targetNode->prev = newNode;
}

/**
 * @brief Removes a node from the freelist.
 * 
 * This function removes a node from the freelist, updating the relevant pointers.
 * If the node to be removed is the head of the list, the head pointer is updated.
 * Handles cases where the node is at the start, middle, or end of the list.
 * 
 * @param nodePtr Pointer to the node to be removed from the freelist.
 */
void remove_freelist_node(void *nodePtr)
{
    FreeListNode *currentNode = FreeListHead;

    // If the node to be removed is the head of the list
    if (currentNode == nodePtr)
    {
        // If there is another node after the head, update the head
        if (currentNode->next != NULL)
        {
            FreeListHead = FreeListHead->next;
            FreeListHead->prev = NULL;
        }
        else
        {
            // If the head is the only node, set the head to NULL
            FreeListHead = NULL;
        }
    }
    else
    {
        currentNode = currentNode->next;

        // Traverse the list to find the node to remove
        while (currentNode != NULL)
        {
            if (currentNode == nodePtr)
            {
                // If the node is not the last node
                if (currentNode->next != NULL)
                {
                    FreeListNode *previousNode = currentNode->prev;
                    currentNode = currentNode->next;
                    previousNode->next = currentNode;
                    currentNode->prev = previousNode;
                }
                else
                {
                    // If the node is the last node, just update the previous node's next pointer
                    currentNode = currentNode->prev;
                    currentNode->next = NULL;
                }
                break;
            }
            currentNode = currentNode->next;
        }
    }
}
/**
 * @brief Searches for the best-fit block in the freelist for a given size.
 *
 * This function searches through the freelist to find the best-fit memory block 
 * for a requested size. It considers both individual blocks and fragmented blocks 
 * (adjacent free blocks) to find the most optimal match. The function returns 
 * a pointer to the allocated memory block.
 *
 * @param requestedSize The size of memory block required.
 * @return void* Pointer to the allocated memory block, or NULL if no suitable block is found.
 */
void *find_best_fit_block(uint64_t requestedSize)
{
    // Variables used for fragmentation handling
    uint8_t fragmentCount = 0;
    uint8_t isFragment = 0;
    uint64_t currentFreeSize = 0;
    uint64_t minFragmentSize = 0;
    FreeListNode *fragmentedBlocks[10000] = {0};
    FreeListNode *bestFragmentedBlocks[10000] = {0};

    // Variables for tracking the best-fit block
    uint8_t foundMinBlock = 0;
    void *allocatedBlock = NULL;
    FreeListNode *bestFitBlock = NULL;
    FreeListNode *currentNode = FreeListHead;
    uint64_t minBlockSize = 0;

    /*******************************************************************************/
    /*                  Search for the minimum suitable block (Best Fit)           */
    /*******************************************************************************/
    while (currentNode != NULL)
    {
        // Identify the smallest block that fits the requested size
        if (currentNode->length >= requestedSize && foundMinBlock == 0)
        {
            minBlockSize = currentNode->length;
            bestFitBlock = currentNode;
            foundMinBlock = 1;
        }
        else if (foundMinBlock == 1 && currentNode->length < minBlockSize && currentNode->length >= requestedSize)
        {
            minBlockSize = currentNode->length;
            bestFitBlock = currentNode;
        }
        currentNode = currentNode->next;
    }

    /***************************************************************************/
    /*                          Fragmentation (Best Fit)                       */
    /***************************************************************************/
    currentNode = FreeListHead;
    while (currentNode != NULL)
    {
        isFragment = 0;
        if (fragmentCount == 0)
        {
            currentFreeSize += currentNode->length;
            fragmentedBlocks[fragmentCount++] = currentNode;
        }
        else
        {
            // Check if the current node is contiguous with the previous one
            if (currentNode == (void *)(currentNode->prev) + (currentNode->prev->length + sizeof(FreeListNode)))
            {
                currentFreeSize += (currentNode->length + sizeof(FreeListNode));
                fragmentedBlocks[fragmentCount++] = currentNode;
            }
            else
            {
                // Reset if not contiguous
                fragmentCount = 0;
                currentFreeSize = 0;
                currentFreeSize += currentNode->length;
                fragmentedBlocks[fragmentCount++] = currentNode;
            }
        }

        // Update the best fragmented block sequence if a better fit is found
        if (currentFreeSize >= requestedSize && fragmentCount > 1 && minFragmentSize == 0)
        {
            isFragment = 1;
            minFragmentSize = currentFreeSize;
            memcpy(bestFragmentedBlocks, fragmentedBlocks, sizeof(fragmentedBlocks));
            fragmentCount = 0;
            currentFreeSize = 0;
        }
        else if (currentFreeSize >= requestedSize && currentFreeSize < minFragmentSize && fragmentCount > 1)
        {
            isFragment = 1;
            minFragmentSize = currentFreeSize;
            memcpy(bestFragmentedBlocks, fragmentedBlocks, sizeof(fragmentedBlocks));
            fragmentCount = 0;
            currentFreeSize = 0;
        }

        if (!isFragment)
        {
            currentNode = currentNode->next;
        }
    }

    /***********************************************************************/
    /*               Determine the most optimal memory block               */
    /***********************************************************************/
    if (minFragmentSize != 0 && foundMinBlock == 1)
    {
        // Choose the smaller between the best fit and the best fragmented fit
        if (minFragmentSize < minBlockSize)
        {
            int i = 0;
            allocatedBlock = bestFragmentedBlocks[0];
            *(size_t *)(allocatedBlock) = requestedSize;
            while (bestFragmentedBlocks[i] != NULL)
            {
                remove_freelist_node(bestFragmentedBlocks[i]);
                i++;
            }
            size_t remainingSize = minFragmentSize - requestedSize;
            if (remainingSize > sizeof(FreeListNode))
            {
                FreeListNode *newNode = (FreeListNode *)((void *)allocatedBlock + requestedSize + sizeof(FreeListNode));
                *(size_t *)newNode = remainingSize - sizeof(FreeListNode);
                insert_block_into_freelist((void *)newNode + sizeof(FreeListNode));
            }
        }
        else
        {
            allocatedBlock = bestFitBlock;
            remove_freelist_node(allocatedBlock);
            size_t remainingSize = bestFitBlock->length - requestedSize;
            if (remainingSize > sizeof(FreeListNode))
            {
                *(size_t *)(allocatedBlock) = requestedSize;
                FreeListNode *newNode = (FreeListNode *)((void *)allocatedBlock + requestedSize + sizeof(FreeListNode));
                *(size_t *)newNode = remainingSize - sizeof(FreeListNode);
                insert_block_into_freelist((void *)newNode + sizeof(FreeListNode));
            }
        }
    }
    else if (minFragmentSize != 0)
    {
        // If no single block is found, but fragmented blocks are
        uint32_t i = 0;
        allocatedBlock = bestFragmentedBlocks[0];
        *(uint64_t *)(allocatedBlock) = requestedSize;
        while (bestFragmentedBlocks[i] != NULL)
        {
            remove_freelist_node(bestFragmentedBlocks[i]);
            i++;
        }
        uint64_t remainingSize = minFragmentSize - requestedSize;
        if (remainingSize > sizeof(FreeListNode))
        {
            FreeListNode *newNode = (FreeListNode *)((void *)allocatedBlock + requestedSize + sizeof(FreeListNode));
            *(uint64_t *)newNode = remainingSize - sizeof(FreeListNode);
            insert_block_into_freelist((void *)newNode + sizeof(FreeListNode));
        }
    }
    else if (foundMinBlock == 1)
    {
        // If a best fit block is found but no fragmented blocks
        allocatedBlock = (void *)bestFitBlock;
        remove_freelist_node(allocatedBlock);
        uint64_t remainingSize = bestFitBlock->length - requestedSize;
        if (remainingSize > sizeof(FreeListNode))
        {
            *(uint64_t *)(allocatedBlock) = requestedSize;
            FreeListNode *newNode = (FreeListNode *)((void *)allocatedBlock + requestedSize + sizeof(FreeListNode));
            *(uint64_t *)newNode = remainingSize - sizeof(FreeListNode);
            insert_block_into_freelist((void *)newNode + sizeof(FreeListNode));
        }
    }

    return (void *)((char *)allocatedBlock + sizeof(FreeListNode));
}

