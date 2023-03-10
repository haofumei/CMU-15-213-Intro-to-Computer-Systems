/*
 * mm-explicit.c
 * use explicit free list: 
 * --------------------
 * | block size | a/f | header
 * --------------------
 * | predecessor      |
 * --------------------
 * | successor        |
 * --------------------
 * | playload         |
 * --------------------
 * | block size | a/f | footer
 * --------------------
 * with heap list:
 *------------------   ---------------------------------   --------------   ---------------*
 | alignment padding | prologue header | prologue footer | regular blocks | epilogue header | 
 *------------------   ---------------------------------   --------------   ---------------*
 * with LIFO list and first fit policy:
 *----------------       -------------      ------
 | free block ptr | <-> | free blocks | -> | tail | 
 *----------------       -------------      ------
 * 
 * add
 * insert_head:
 * 1. insert the free block into LIFO list
 * remove_block:
 * 1. remove block from LIFO list
 * mm_checkfree:
 * 1. check the LIFO list
 * 
 * modify
 * mm_init: 
 * 1. fbp points to tail, tail points to fbp
 * mm_malloc: 
 * 1. remove free block while place
 * mm_free:
 * 1. add free block to LIFO list
 * mm_realloc: 
 * 1. if oldsize >= newsize, return
 * 2. merge next block if possible
 * find_fit:
 * 1. search from LIFO list
 * extend_heap:
 * 1. add the extended block to LIFO list
 * coalesce:
 * case 1: both allocated, case 2: next is not allocated
 * case 3: prev is not allocated, case 4: both are not allocated
 * 1. case 1: return
 * 2. case 2: remove next
 * 3. case 3: remove prev
 * 4. case 4: remove next and prev
 * place:
 * 1. add splited free block into LIFO list if split
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "self study",
    /* First member's full name */
    "Haofu, Mei",
    /* First member's email address */
    "haofumei@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};
/* 
 * not avaliable for explicit list
 */
//#define NEXT_FIT 0

/* checkheap open or close */
//#define checkheap(void) mm_checkheap(void)
#define checkheap(void)

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 6)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* read and write a word at address p */
#define GET(p) (* (unsigned int *) (p))
#define PUT(p, val) (* (unsigned int *) (p) = (val))

/* get the size and allocated bit from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* given block ptr bp, compute the address of next and prev blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* give address p, compute the address that it is pointing to */
#define GET_ADDR(p) (* (void **) p)

/* give block ptr bp, compute the address of pred and succ */
#define GET_PRED(bp) (GET_ADDR(bp))
#define GET_SUCC(bp) (GET_ADDR(((char *) (bp) + DSIZE)))
/* give block ptr bp and address of pred or succ, witre the address into block */
#define SET_PRED(bp, pred) (GET_PRED(bp) = (pred))
#define SET_SUCC(bp, succ) (GET_SUCC(bp) = (succ))

/* pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* private variables */
static char *hp; /* points to first byte of heap list */
static void *fbp; /* points to the head of LIFO list */
static void *tail; /* tail of LIFO list */

#ifdef NEXT_FIT
static char *rover; /* points to last next fit block */
#endif

/* Helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
static void place(char *bp, size_t size);
static inline void insert_head(void *bp);
static inline void remove_block(void *bp);
void mm_checkheap(void);
void mm_checkfree(void);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((hp = mem_sbrk(4 * WSIZE)) == (void *) - 1)
        return -1;
    PUT(hp, 0); /* alignment padding */
    PUT(hp + (1*WSIZE), PACK(DSIZE, 1)); /* prologue header */
    PUT(hp + (2*WSIZE), PACK(DSIZE, 1)); /* prologue footer */
    PUT(hp + (3*WSIZE), PACK(0, 1)); /* epilogue header */
    hp += DSIZE; 
    
    fbp = &tail; /* set up the head and tail of LIFO */
    tail = &fbp; 

    #ifdef NEXT_FIT
    rover = hp;
    #endif

    /* extend the empty heap with a free block of CHUNKSIZE bytes */
    if ((fbp = extend_heap(CHUNKSIZE / WSIZE)) == NULL)
        return -1;
    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;

    if (size == 0) /* ignore spurious requests */
        return NULL;

    if (size <= DSIZE) { /* adjust the size to include header, footer and alignment reqs */
        asize = 2 * DSIZE;
    } else {
        asize = ALIGN(size + DSIZE);
    }

    if ((bp = find_fit(asize)) != NULL) { /* search the free slot to place*/
       remove_block(bp);
	    place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
        
    remove_block(bp);
    place(bp, asize);
    return bp;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0)); 
    PUT(FTRP(ptr), PACK(size, 0));  
    ptr = coalesce(ptr);
    insert_head(ptr);
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    size_t asize;
    void *newptr;

    if (size == 0) { /* if size is 0, just call mm_free(ptr) */
    	mm_free(ptr);
    	return NULL;
    }
    
    if (ptr == NULL) { /* it is equivalent to mm_malloc(size) */
    	return mm_malloc(size);
    }
    
    asize = ALIGN(size + DSIZE);
    oldsize = GET_SIZE(HDRP(ptr));
    
    if (asize <= oldsize) { 
    	return ptr;
    }
    
    /* only merge next if possible */
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    if (!next_alloc && (oldsize + next_size) >= asize) {
        remove_block(NEXT_BLKP(ptr));
        PUT(HDRP(ptr), PACK((oldsize+next_size), 1));
        PUT(FTRP(ptr), PACK((oldsize+next_size), 1));
        return ptr;
    }
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
	return NULL;
    memcpy(newptr, ptr, oldsize);
    mm_free(ptr);
    return newptr;
    
    
    /* merge next and prev if possible (lower thru)
    newptr = coalesce(ptr);
    
    if (GET_SIZE(HDRP(newptr)) < asize) {
    	void *allocptr = mm_malloc(size);
	if (allocptr == NULL)
	    return NULL;
	memcpy(allocptr, ptr, oldsize);
	mm_free(newptr);
	return allocptr;
    }
    
    memmove(newptr, ptr, oldsize);
    place(newptr, asize);
    return newptr; */
}

/* 
 * extend heap list by the size of the even words
*/
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; /* allocate an even number */
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;
    PUT(HDRP(bp), PACK(size, 0)); /* free block header */
    PUT(FTRP(bp), PACK(size, 0)); /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */
    bp = coalesce(bp); /* coalesce if the prev block is free */
    insert_head(bp);
    return bp; 
}

/*
 * coalesce the prev and next block of the current block if possible 
*/
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* both prev and next blocks are allocated */
        return bp;
    }
    else if (prev_alloc && !next_alloc) { /* prev block is allocated while next is not */
        remove_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) { /* next block is allocated while prev is not */
        remove_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else { /* both prev and next blocks are not allocated */
        remove_block(PREV_BLKP(bp));
        remove_block(NEXT_BLKP(bp));
        size += GET_SIZE(FTRP(NEXT_BLKP(bp)));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    #ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) 
        rover = bp;
    #endif
    
    return bp;
}

/* 
 * find the avaliable slot of size in heap
 * if no available slot, return NULL
*/
static void *find_fit(size_t size)
{
    #ifdef NEXT_FIT
        /* next fit search */
        char *oldrover = rover;
    	
    	for (; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
            if (!GET_ALLOC(HDRP(rover)) && GET_SIZE(HDRP(rover)) >= size)
            	return rover;
            	
        for (rover = hp; rover < oldrover; rover = NEXT_BLKP(rover))
            if (!GET_ALLOC(HDRP(rover)) && GET_SIZE(HDRP(rover)) >= size)
            	return rover;
            	
        return NULL;
    #else 
        /* first fit search */
        void *cur;
        for (cur = fbp; cur != &tail; cur = GET_SUCC(cur)) {
            if (GET_SIZE(HDRP(cur)) >= size)
                return cur;
        }
        return NULL;
    #endif
}

/*
 * allocate the block of size at the address of bp
 */
static void place(char *bp, size_t size)
{
    size_t diff = GET_SIZE(HDRP(bp)) - size;

    if (diff <= DSIZE * 3) { /* remaining space is not enough to construct a new block */
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
        PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
    } else { /* split the block */
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(diff, 0));
        PUT(FTRP(bp), PACK(diff, 0));
        insert_head(bp);
    }
    checkheap(void);
}

/*
 * insert block bp into the head of LIFO list
 */
static inline void insert_head(void *bp) 
{

    SET_PRED(fbp, bp);
    SET_SUCC(bp, fbp);
    SET_PRED(bp, &fbp);
    fbp = bp;
}

/*
 * remove block bp from LIFO list
 */
static inline void remove_block(void *bp) 
{
    void *pred = GET_PRED(bp);
    void *succ = GET_SUCC(bp);
    SET_PRED(succ, pred);
    if (pred == &fbp) { /* pred is fbp */
        fbp = succ;
    } else {
        SET_SUCC(pred, succ);
    }
}

void mm_checkheap(void)
{
    char *bp = hp;
    printf("******begin of heap******\n");
    while (GET_SIZE(HDRP(bp))) {
        printf("size: %d, allcated: %d, at: %p\n", GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), bp);
	bp = NEXT_BLKP(bp);
    }
    printf("******end of heap******\n");
}

void mm_checkfree(void)
{
    void *cur;
    int i = 0;
    for (cur = fbp; cur != &tail; cur = GET_SUCC(cur), i++) {
    	printf("%d node is %p, pred: %p, succ: %p with size %d\n", i, cur, GET_PRED(cur), GET_SUCC(cur), GET_SIZE(HDRP(cur)));
    }
}










