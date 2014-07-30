/*
 * 
 * Our approach uses a segregated fit free list with first fit searchs. Our free list
 * is an array of 20 segregated free lists. Each list holds freeblocks between a certain 
 * size. List n contains blocks of size 2^n from 2^n + 1. Whenever searching these lists
 * for a free block a first fit search is implemented
 * 
 * mm_malloc(): When a block of size n is requested it is first determined which free 
 * 		list would hold a block of it's size. The resulting free list is then 
 *		searched for a block of at least size n. If no such block is found then
 *		the next list is searched. This is completed until all free lists have 
 *		been searched. If no blocks are found then the heap is extended
 *
 * mm_free(): First the header of the block is found and the reinitialized as a free block.
 * 	      The free block is then merged with it's right and left neighbors if possible.
 *	      once the block is merged it is put onto the correct freelist
 *
 * mm_realloc(): First the header of the block is found. Then the next block is examined to
 *		 see if it is free. If the next block is free realloc determines if the block
 *		 should be extended or shrunk. If it is to be extended then the next free block
 *		 must be large enough to still hold a free block declartion. If it is then the 
 *		 block can be extended into it. If the block is to be shrunk then the adjacent
 *		 free block is to be extended to recover the space lost. If the next block is
 *		 allocated and the block is to be shrunk then the block will be shrunk if the
 *		 lost space is big enough to hold a free block. If all these cases fail then 
 *		 a new pointer of the correct size is created, the payload of the origonal 
 *		 block is copied into it, and the origonal block is freed.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>

#include "mm.h"
#include "memlib.h"
#include "config.h"             /* defines ALIGNMENT */
#include "list.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "group450",
    /* First member's full name */
    "Daniel Moore",
    /* First member's SLO (@cs.vt.edu) email address */
    "dmoore09",
    /* Second member's full name (leave blank if none) */
    "Minahm Kim",
    /* Second member's SLO (@cs.vt.edu) email address (leave blank if none) */
    "minahm92"
};


/* 
 * If size is a multiple of ALIGNMENT, return size.
 * Else, return next larger multiple of ALIGNMENT:
 * (size/ALIGNMENT + 1) * ALIGNMENT
 * Does so without requiring integer division, assuming
 * ALIGNMENT is a power of 2.
 */
static size_t roundup(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

int pow1(int a, int b){
	int i;
	int result = 1;
	for (i = 0; i < b; i++){
		result = result * a;
	}
	return result;
}


//find the appropriate freelist for a size
int findList(size_t size){
    int i;
    for (i = 0; i < 20; i++){
	if ( size <= pow1(2, i + 1) && size > pow1(2, i)){
		return i;	
	}
    }
    return 19;
}

//the header for a free block. Contains a list_elem so it can be added to a
//freelist
struct free_block {
	unsigned int free:1;
   	unsigned int size:31;
	struct list_elem elem;	
};

//the footer of a block. Both allocated and free blocks have footers
//used to coallesce blocks
struct footer {
	unsigned int free:1;
   	unsigned int size:31;
};

/* 
 * This C struct captures an allocated header.
 *
 * By casting a memory location to a pointer to a allocated_block_header,
 * we are able to treat a part of memory as if a header had been allocated
 * in it.
 *
 * Note: you should never define instances of 'struct allocated_block_header' -
 *       all accesses will be through pointers.
 */
struct allocated_block_header {
    unsigned int free:1;
    unsigned int size:31;
    /* 
     * Zero length arrays do not add size to the structure, they simply
     * provide a syntactic form to refer to a char array following the
     * structure.
     * See http://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
     *
     * The 'aligned' attribute forces 'payload' to be aligned at a
     * multiple of alignment, counted from the beginning of the struct
     * See http://gcc.gnu.org/onlinedocs/gcc/Variable-Attributes.html
     */
    char        payload[0] __attribute__((aligned(ALIGNMENT)));
};

struct free_block* mergeFreeBlock(struct free_block* free);
int mm_check(void);
int allocatedSize;
int freeSize;
void resetList(struct list_elem *elem, size_t size);
//20 segregated free lists, list n has blocks of size 2^n from 2^n + 1
static struct list* freeLists[20];
static struct list freeList1;
static struct list freeList2;
static struct list freeList3;
static struct list freeList4;
static struct list freeList5;
static struct list freeList6;
static struct list freeList7;
static struct list freeList8;
static struct list freeList9;
static struct list freeList10;
static struct list freeList11;
static struct list freeList12;
static struct list freeList13;
static struct list freeList14;
static struct list freeList15;
static struct list freeList16;
static struct list freeList17;
static struct list freeList18;
static struct list freeList19;
static struct list freeList20;

//keeps track of greatest size block used to improve allocation preformance
size_t greatestSize;

/* 
 * mm_init - initialize free lists and other global variables
 */
int mm_init(void)
{
    allocatedSize = roundup(sizeof(struct allocated_block_header)+sizeof(struct footer));
    freeSize = sizeof(struct footer) + sizeof(struct free_block);
    /* Sanity checks. */
    assert((ALIGNMENT & (ALIGNMENT - 1)) == 0); // power of 2
    //initialize the free lists
    //int listSize = sizeof(struct list);
    int i;
    freeLists[0] = &freeList1;
    freeLists[1] = &freeList2;
    freeLists[2] = &freeList3;
    freeLists[3] = &freeList4;
    freeLists[4] = &freeList5;
    freeLists[5] = &freeList6;
    freeLists[6] = &freeList7;
    freeLists[7] = &freeList8;
    freeLists[8] = &freeList9;
    freeLists[9] = &freeList10;
    freeLists[10] = &freeList11;
    freeLists[11] = &freeList12;
    freeLists[12] = &freeList13;
    freeLists[13] = &freeList14;
    freeLists[14] = &freeList15;
    freeLists[15] = &freeList16;
    freeLists[16] = &freeList17;
    freeLists[17] = &freeList18;
    freeLists[18] = &freeList19;
    freeLists[19] = &freeList20;
    for (i = 0; i < 20; i++){
	list_init(freeLists[i]);
    }

    greatestSize = 0;
    return 0;
}

/**
 * *Resets the list of the adjusted free block to the appropriate block
 * *Param: pos is position of the free_block
 * *Param: size of the free block to be reset
 * */
void resetList(struct list_elem *elem, size_t size){
	int newList = findList(size);
	list_push_back(freeLists[newList],elem);
}


/* 
 * Allocate a block of size n. first perform a first fit search in the appropriate free
 * lists. If no free blocks of a great enough size are found then extend the heap
 */
void *mm_malloc(size_t size)
{
   //search through free list, if a block of big enough size is found, return it's payload
   //if size is not multiple of alignment, it changes the block size to fit.
   struct list_elem* e; 
   if(size%ALIGNMENT!=0){
	size = size + (ALIGNMENT-size%ALIGNMENT);
   }
   //find appropriate segregated list
   int listNum = findList(size);
   int i;

   if (size < greatestSize){
   for (i = listNum; i < 20; i++){
  	//get the a free list	
  	struct list* freeList = freeLists[i];	
   
  	//check free blocks
  	for(e = list_begin (freeList); e != list_end (freeList); e = list_next(e)){

   	  	struct free_block *f = list_entry (e, struct free_block, elem);
		if(f->size == size + allocatedSize){
			list_remove(e);
			struct allocated_block_header * allocated = (struct allocated_block_header *)f;
			//initialize footer			
			struct footer* newFooter = (void*)allocated + size + allocatedSize - 4;
			newFooter->size = size;
			newFooter->free = 0;
			allocated->size = size;
			allocated->free = 0;
			return allocated->payload;
		}	
		if(f->size > size + roundup(sizeof(struct allocated_block_header) + sizeof(struct free_block) + 2*sizeof(struct 					footer))){
			struct allocated_block_header * allocated;
       			allocated = (void*)f + (f->size - (size + allocatedSize));
			struct footer* allocatedFooter = (void*)allocated + size +  allocatedSize- 4;
			allocated->size = size;
			allocated->free = 0;
			allocatedFooter->size = size;
			allocatedFooter->free = 0;

       			//set new size of free block and put it in appropriate freeList
			f->size = f->size - (size + allocatedSize);
			f->free = 1;
			struct footer* freeFooter = (void*)f + f->size - 4;
			freeFooter->size = f->size;
			freeFooter->free = 1;
			list_remove(e);
			resetList(e,f->size);
			return allocated->payload;
		}
   	 }
    }
    }
 
    //no blocks can satisfy request, extend the heap
    int newsize = size + allocatedSize;
    struct allocated_block_header * blk = mem_sbrk(newsize);
    struct footer* newFooter = (void*)blk + newsize - 4;
    if (blk == NULL){
	return NULL;
    }
    blk->size = size;
    newFooter->size = size;
    blk->free = 0;
    newFooter->free = 0;
    return blk->payload;
}

/*
 * mm_free - create a free block pointing to the address of the header of the old block
 * 	     , coalesce it and put it on the free list
 */
void mm_free(void *ptr)
{
	//turn pointer into a free block
        struct free_block* newFree = ptr - offsetof(struct allocated_block_header, payload);
	newFree->size = newFree->size + allocatedSize;
	newFree->elem.next = NULL;
	newFree->elem.prev = NULL;
	newFree->free = 1;
	struct footer* newFooter = (void*)newFree + newFree->size - 4;
	newFooter->size = newFree->size;
	newFooter->free = 1;
	struct free_block* merged = mergeFreeBlock(newFree);
	//find the correct segregated list to insert into
	//struct free_block* merged = newFree;
	resetList(&merged->elem, merged->size);

	//keep track of the greatest size free block
	if (merged->size > greatestSize){
		greatestSize = merged->size;
	}
}
	


/*
 * mm_realloc - changes to size of an already allocated block whose payload is
 * pointed at by *oldptr. First checks if the next block is free and trys to satisfy
 * request. Then it checks to see if the block is big engough to satisfy the request itself.
 * if not a new block is allocated and the contents of the origonal block are copied and it
 * is freed
 */
void *mm_realloc(void *oldptr, size_t size)
{
    if (oldptr == NULL){
	return mm_malloc(size);
    }
    
    if (size == 0){
	mm_free(oldptr);
	return NULL;
    }

    //adjust size
    size = roundup(size);

    /* Assuming 'oldptr' was a '&payload[0]' in an allocated_block_header,
     * determine its start as 'oldblk'.  Then its size can be accessed
     * more easily.
     */
    struct allocated_block_header *oldblk;
    oldblk = oldptr - offsetof(struct allocated_block_header, payload);
    //get the footer of old block
    struct footer* oldfoot;

    if (size == oldblk->size){
	return oldptr;
    }

    //check if next block is a free block
    struct free_block* next = (void*)oldblk + oldblk->size + allocatedSize;
    if(next->free == 1 && (void*)next < mem_heap_hi()){
	//next block is free
	if (size > oldblk->size){
		int check = next->size - (size - oldblk->size);
		//block needs to grow by make sure free block is large enough
		if ( check > 16){
			size_t oldSize = next->size;
			//move next block forward
			list_remove(&next->elem);
			next = (void*)next + (size - oldblk->size);
			//reset size and insert into correct list
			next->size = oldSize - (size - oldblk->size);
			next->free = 1;
			resetList(&next->elem, next->size);

			//reset footer for free block
			struct footer* fFoot = (void*)next + next->size - 4;
			fFoot->size = next->size;
			fFoot->free = 1;

			oldblk->size = size;
    			oldfoot = (void*)oldblk + oldblk->size + allocatedSize - 4;
			oldfoot->size = size;
			oldfoot->free = 0;
			return oldblk->payload;
		}
		else if (check == 0){
			list_remove(&next->elem);
			oldblk->size = size;
    			oldfoot = (void*)oldblk + oldblk->size + allocatedSize - 4;
			oldfoot->size = size;
			oldfoot->free = 0;
			return oldblk->payload;
		}
	}
	else{
		//block needs to shrink move next back
		list_remove(&next->elem);
		size_t oldSize = next->size;
		next = (void*)next - (size - oldblk->size);
		next->size = oldSize + (size - oldblk->size);
		next->free = 1;
		int i = findList(next->size);
		list_push_back(freeLists[i], &next->elem);
		//update footer of free block
		struct footer* fFoot = (void*)next + next->size - 4;
		fFoot->size = next->size;
		fFoot->free = 1;

		//update allocated header and footer
		oldblk->size = size;
		oldfoot = (void*)oldblk + oldblk->size + allocatedSize- 4;
		oldfoot->size = size;
		oldfoot->free = 0;
		return oldblk->payload;
	}
    }

    //see if there is enough room to split when next block is not free
    if (oldblk->size - size > (freeSize) && size < oldblk->size){
		struct free_block* splitBlock = (void*)oldblk + allocatedSize + size;
		splitBlock->size = oldblk->size - size;
		splitBlock->free = 1;
		resetList(&splitBlock->elem, splitBlock->size);
		//update free block footer
		struct footer* fFoot = (void*)splitBlock + next->size - 4;
		fFoot->size = next->size;
		fFoot->free = 1;

		//update allocated header and footer
		oldblk->size = size;
		oldfoot = (void*)oldblk + oldblk->size + allocatedSize - 4;
		oldfoot->size = size;
		oldfoot->free = 0;
		return oldblk->payload;
    }
    if (oldblk->size - size < (freeSize) && size < oldblk->size){
		return oldblk->payload;
    }

    //size needs to be changed
    void *newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;


    size_t copySize = oldblk->size;
    if (size < copySize){
      copySize = size;
    }	

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/* merge the free block with previous free block. Testing found it did not effect util
 * and imporved performance to only merge with the previous block
*/
struct free_block* mergeFreeBlock(struct free_block* free){
	
	//faster
	/*void* end = mem_heap_hi();
	//check boundary conditions
	//check next block
	struct free_block* nextBlock = (void*)free + free->size;
	void* checkBounds = (void*)nextBlock + nextBlock->size;
	if (nextBlock->free == 1  && checkBounds < end){
		free->size = free->size + nextBlock->size;
		//declare a new footer
		struct footer* newFooter = (void*)free + free->size - 4;
		newFooter->size = free->size;
		newFooter->free = 1;
		list_remove(&nextBlock->elem);
	}*/	

	void* start = mem_heap_lo();

	struct footer* foot = (void*)free - 4;
	if (foot->free == 1 && (void*)foot >= start){
		//get previous block
		struct free_block* prevBlock = (void*)foot + 4 - (foot->size);
		//merge with current
		prevBlock->size = prevBlock->size + free->size;
		struct footer* newFooter = (void*)prevBlock + prevBlock->size - 4;
		newFooter->size = prevBlock->size;
		newFooter->free = 1;
	
		list_remove(&prevBlock->elem);
		return prevBlock;
	}
	return free;
}

/*
 * checks for various heap inconsistencies. If an error is found the associated addresses and values are printed out
 * 	- makes sure header and footer info matches
 *	- makes sure free block list elements are valid
 *	- makes sure blocks dont overlap
*/	
int mm_check(void){
	void* start = mem_heap_lo();
	void* end = mem_heap_hi();
	struct free_block* block = start;
	struct footer* footer;
	struct free_block* lastblock;
	while(start < end){
		lastblock = block;
		if (block->free == 1){
			//block is free
			void* move = block;
			move = move + block->size;
			block = move;
			footer = move + block->size -4;
		}
		else{	
			//block is allocated
			void* move = block;
			move = move + block->size + allocatedSize;
			block = move;
			footer = move +allocatedSize + block->size - 4;
		}
		//make sure footer and header agree on type
		if (footer->free != block->free){
			if (block->free == 0){
				printf("allocated footer thinks it is free at: %p and %p\n", block, footer);
			}
			else{
				printf("free footer thinks it is allocated at: %p and %p\n", block, footer);
			}
		}
		//make sure footer and head agree on size
		if (footer->size != block->size){
			printf("footer and header have differect sizes at: %p and %p, sizes: %d, %d\n", block, footer, block->size, 					footer->size);
		}
		//check free block elem
		if (block->free == 1){
			if (block->elem.next == block->elem.prev){
				printf("block pointers point at each other next: %p, prev: %p", block->elem.next, block->elem.prev);
			}
		}
		if ((void*)lastblock + lastblock->size > (void*)block){
			printf("blocks overlap %p and %p", lastblock, block);
		}	
	}
	printf("done memcheck\n"); 
	return 1;
}

