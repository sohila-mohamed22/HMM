#ifndef FreeList
#define FreeList
// Define the FreeListNode structure
typedef struct FreeListNode {
    uint64_t length;
    struct FreeListNode *prev;
    struct FreeListNode *next;
} FreeListNode;

// Function declarations
uint8_t calculate_decreases_in_program_break();
void insert_block_into_freelist(void *blockPtr);
void insert_node_at_start(void *ptr);
void append_to_freelist_end(void *blockPtr);
void insert_node_between(FreeListNode *currentNodePtr, void *blockPtr);
void remove_freelist_node(void *nodePtr);
void *find_best_fit_block(uint64_t requestedSize);
#endif
