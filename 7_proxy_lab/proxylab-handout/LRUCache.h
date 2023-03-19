#ifndef __LRUCACHE_H__
#define __LRUCACHE_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned char byte;

typedef struct node
{
    int len; // length of val
    char *key;
    byte *val;
    struct node *prev;
    struct node *next;
} Node;

typedef struct lrucache
{
    int size;
    int capacity; 
    Node *head;
    Node *tail;
} LRUCache;

LRUCache *initLRUCache(int capacity);
void freeCache(LRUCache *cache);
void put(LRUCache *cache, char *key, byte *val, int len);
Node *get(LRUCache *cache, char *key);
void printCache(LRUCache* cache);

#endif