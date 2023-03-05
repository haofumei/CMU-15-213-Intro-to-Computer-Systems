#include <stdio.h>
#include <stdlib.h>

/* read and write a word at address p */
#define GET(p) (* (unsigned int *) (p))
#define PUT(p, val) (* (unsigned int *) (p) = (val))
#define GET_LIST_PTR(bp)        (*(void**) bp)


/* give block ptr bp, commpute the address of pred and succ blocks */
#define GET_ADDR(bp) (* (void **) bp)

/* give block ptr bp, compute the address of pred and succ */
#define GET_PRED(bp) (GET_ADDR(bp))
#define GET_SUCC(bp) (GET_ADDR(((char *) (bp) + 8)))

#define SET_PRED(bp, pred) (GET_PRED(bp) = (pred))
#define SET_SUCC(bp, succ) (GET_SUCC(bp) = (succ))

static inline void insert_head(void *bp);
static inline void remove(void *bp);

int main()
{
    int *tail = tail;
    int *fbp = (int *) malloc(100 * sizeof(int));
    
    SET_PRED(fbp, &fbp);
    SET_SUCC(fbp, &tail);

    

    printf("address of %p\n", head);
    printf("address of %p\n", GET_ADDR(head));
    /*
    int *t = GET_PRED(p);
    SET_PRED(t, p);


    int *cur = p;
    for (int i = 0; i < 50; i++) {
        printf("%p 's next node is %p\n", cur, GET_ADDR(cur));
        cur = GET_PRED(cur);
    }*/


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
static inline void remove(void *bp) 
{
    void *pred = GET_PRED(bp);
    void *succ = GET_SUCC(bp);
    SET_PRED(succ, pred);
    SET_SUCC(pred, succ);
}