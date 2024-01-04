#include "mem.h"                      
extern BLOCK_HEADER* first_header;

// return a pointer to the payload
// if a large enough free block isn't available, return NULL
void* Mem_Alloc(int size){
	BLOCK_HEADER *current = first_header;
    // find a free block that's big enough
	while (size > current->size_alloc - 8 || current->size_alloc % 16) {
		if (current->size_alloc == 1) {
			return NULL;
		}
		current += current->size_alloc / sizeof(current);
	}
    // return NULL if we didn't find a block
	
    // allocate the block
	int excessPadding = current->size_alloc - size - 8 - (16 * ((size + 7) / 16) + 16 - (size + 8));
	current->size_alloc++;
	current->payload = size;
	int *ptr = (int*)current;
	ptr += 2;

    // split if necessary (if padding size is greater than or equal to 16 split the block)
	if (excessPadding >= 16) {
		current->size_alloc -= excessPadding;
		current += current->size_alloc / sizeof(current);
		BLOCK_HEADER new;
		new.size_alloc = excessPadding;
		new.payload = excessPadding - 8;
		*current = new;
	}
    return ptr;
}


// return 0 on success
// return -1 if the input ptr was invalid
int Mem_Free(void *ptr){
    // traverse the list and check all pointers to find the correct block 
    // if you reach the end of the list without finding it return -1
	BLOCK_HEADER *current = first_header;
	BLOCK_HEADER *prev = first_header;
	BLOCK_HEADER *next = first_header;
	while (current + 1 != ptr) {
		if (current->size_alloc == 1) {
			//printf("failed\n");
			return -1;
		}
		prev = current;
		current += current->size_alloc / sizeof(current);
	}		
	next = current + (current->size_alloc / sizeof(current));
    // free the block 
	current->size_alloc--;
	current->payload = current->size_alloc - 8;
    // coalesce adjacent free blocks
	if (!(next->size_alloc % 16)) {
		current->size_alloc += next->size_alloc;
		current->payload += next->size_alloc;
		next->size_alloc = 0;
		next->payload = 0;
	}
	if (prev != current && !(prev->size_alloc % 16)) {
		prev->size_alloc += current->size_alloc;
		prev->payload += current->size_alloc;
		current->size_alloc = 0;
		current->payload = 0;
	}
    return 0;
}

