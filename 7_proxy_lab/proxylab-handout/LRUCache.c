#include "LRUCache.h"

/*
 * Create a new node with the given key and value 
 */
static Node *createNode(char *key, byte *val, int len) {
    Node *newNode = (Node *) malloc(sizeof(Node));

    newNode->key = (char *) malloc(strlen(key) + 1); // store the key
    strcpy(newNode->key, key);

    newNode->len = len;
    newNode->val = (byte *) malloc(newNode->len); // store the val
    memcpy(newNode->val, val, len);

    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

/*
 * Free the node
 */
static void freeNode(Node *node) {
    free(node->key);
    free(node->val);
    free(node);
}

/*
 * Initialize the cache
 */
LRUCache *initLRUCache(int capacity) {
    LRUCache *newCache = (LRUCache *) malloc(sizeof(LRUCache));
    newCache->size = 0;
    newCache->capacity = capacity;
    newCache->head = NULL;
    newCache->tail = NULL;
    return newCache;
}

/*
 * Free the cache
 */
void freeCache(LRUCache *cache) {
    Node *node = cache->head, *temp;
    while (node) {
        temp = node;
        node = node->next;
        freeNode(temp);
    }
    free(cache);
}

/*
 * Move the given node to the front of the cache
 */
inline static void moveToHead(LRUCache *cache, Node *node) {
    if (node == cache->head) /* node is in the head */
        return;
    if (node == cache->tail) { /* node is in the tail */
        cache->tail = node->prev;
        cache->tail->next = NULL;
    } else { /* node is in the middle */
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    node->next = cache->head;
    node->prev = NULL;
    cache->head->prev = node;
    cache->head = node;
}

/*
 * Remove the least recently used node from the cache
 */
static void removeNode(LRUCache *cache) {
    Node *node = cache->tail;
    if (node->len == cache->size) { /* only one node in the cache */
        cache->head = NULL;
        cache->tail = NULL;
    } else {
        cache->tail = node->prev;
        cache->tail->next = NULL;
    }
    cache->size -= node->len;
    freeNode(node);
}

/*
 * Add a key-value pair to the cache
 */
void put(LRUCache *cache, char *key, byte *val, int len)
{
    Node *node = createNode(key, val, len); 

    while ((node->len + cache->size) > cache->capacity) /* if not enough space in cache */
        removeNode(cache);
    
    if (cache->head) {
        node->next = cache->head;
        cache->head->prev = node;
        cache->head = node;
    } else { /* empty cache */
        cache->head = node;
        cache->tail = node;
    }
    cache->size += node->len;
}

/*
 * Get the Node associated with the given key from the cache
*/
Node *get(LRUCache *cache, char *key) {
    Node *node = cache->head;
    while (node) { 
        if (!strcmp(node->key, key)) {
            moveToHead(cache, node);
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/*
 * Print the contents of the cache
 */ 
void printCache(LRUCache* cache) {
    Node* node = cache->head;
    printf("Cache contents with size %d: \n", cache->size);
    while (node) {
        printf("(%s, size: %d)\n", node->key, node->len);
        node = node->next;
    }
}

