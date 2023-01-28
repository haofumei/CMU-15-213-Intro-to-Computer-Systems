#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>

// Structure
typedef struct cache_line {
    uint64_t tag; // Cache tag
    int counter; // Imply priority of eviction
    int valid; // Valid bit
} cache_line_t;

typedef cache_line_t *cache_line_ptr;

// Function
void print_usage();  // Print usage info

void parse_command_line(int argc, char** argv, int* s, int* E, int* b, char* trace, int* verbose);
/* Parse the command line and set up the parameters s, E, b, trace, verbose.
 - argc: Size of input string.
 - argv: Input string.
 - s: Number of set index bits.
 - E: Associativity.
 - b: Number of block bits.
 - trace: Name of trace file.
 - verbose: Optional verbose flag that displays trace info.
*/

cache_line_ptr* init_cache(int s, int E); 
/* Allocate memory for cache.
 - cache: Pointer holds the first bit of cache.
 - s: Number of set index bits.
 - E: Associativity.
*/

void free_cache(cache_line_ptr* cache, int s);
/* Free the allocated memory for cache.
 - cache: Pointer holds the first bit of cache.
 - s: Number of set index bits.
*/

void read_trace(cache_line_ptr* cache, char* trace, int s, int E, int b, int verbose, int* records);
/* Simulate cache hit and miss, ignore all instruction cache accesses (lines starting with “I”). 
 * Recall that valgrind always puts “I” in the first column (with no preceding space), and “M”, “L”, and “S” in the second column (with a preceding space).
 - cache: Pointer holds the first bit of cache.
 - trace: Name of trace file.
 - s: Number of set index bits.
 - E: Associativity.
 - b: Number of block bits.
 - verbose: Optional verbose flag that displays trace info.
 - records: Array[0: number of hit, 1: number of miss, 2: number of eviction, 3: counter].
*/

void update_cache(cache_line_ptr lines, uint64_t tag, int* records, int E, int verbose);
/* Simulate cache access by LRU, and record hit, miss, eviction into records.
 - lines: Current access cache set.
 - tag: Cache line tag.
 - records: Array hold the number of hit, miss, eviction.
 - E: Associativity.
 - verbose: Optional verbose flag that displays trace info.
*/


int main(int argc, char** argv) {
    int s = 0, E = 0, b = 0, verbose = 0;
    char trace[20];
    int *records = calloc(4, sizeof(int));
    parse_command_line(argc, argv, &s, &E, &b, trace, &verbose);
    cache_line_ptr *cache = init_cache(s, E);
    read_trace(cache, trace, s, E, b, verbose, records);
    //printf("hit: %d, miss: %d, evic: %d\n", records[0], records[1], records[2]);
    printSummary(records[0], records[1], records[2]);

    free_cache(cache, s);
    free(records);
    return 0;
}

void print_usage() {
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    printf("\t-h: Optional help flag that prints usage info\n");
    printf("\t-v: Optional verbose flag that displays trace info\n");
    printf("\t-s <s>: Number of set index bits (S = 2^s is the number of sets)\n");
    printf("\t-E <E>: Associativity (number of lines per set)\n");
    printf("\t-b <b>: Number of block bits (B = 2^b is the block size)\n");
    printf("\t-t <tracefile>: Name of the valgrind trace to replay\n");
}

void parse_command_line(int argc, char** argv, int* s, int* E, int* b, char* trace, int* verbose) {
    int option;
    while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (option)
        {
        case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
        case 'v':
            *verbose = 1;
            break;
        case 's':
            *s = atoi(optarg);
            break;
        case 'E':
            *E = atoi(optarg);
            break;
        case 'b':
            *b = atoi(optarg);
            break;
        case 't':
            strncpy(trace, optarg, 20);
            break;
        default:
            printf("Wrong command! Please usage info by\n");
            printf("./csim-ref -h\n");
            exit(EXIT_FAILURE);
        }
    }
}

cache_line_ptr* init_cache(int s, int E) {
    int num_s = 1 << s;
    cache_line_ptr* cache;
    if ((cache = calloc(num_s, sizeof(cache_line_ptr))) == NULL) {
        perror("calloc sets failed");
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < num_s; i++) {
            if ((cache[i] = calloc(E, sizeof(cache_line_t))) == NULL) {
                perror("calloc lines failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    return cache;
}

void free_cache(cache_line_ptr* cache, int s) {
    int num_s = 1 << s;
    for (int i = 0; i < num_s; i++) {
        free(cache[i]);
    }
    free(cache);
}

void read_trace(cache_line_ptr* cache, char* trace, int s, int E, int b, int verbose, int* records) {
    FILE* file = fopen(trace, "r");
    if (!file) {
        perror("Can not open the file");
        exit(EXIT_FAILURE);
    }
    // Line example: " M 0400d7d4,8"
    char identifier;
    uint64_t address;
    int size;
    while (fscanf(file, " %c %lx,%d", &identifier, &address, &size) > 0) {
        if (identifier == 'I') continue; // Filter possible wrong data.

        // Extract tag, set index
        uint64_t mask = (1 << s) - 1;
        uint64_t set_index = (address >> b) & mask;
        uint64_t tag = address >> (s + b);
        records[3] += 1; // Update counter
        
        // 'L' or 'S'
        if (verbose) printf("%c %lx,%d ",identifier, address ,size);
        update_cache(cache[set_index], tag, records, E, verbose);

        // 'M' equals to 'L' + 'S'
        if (identifier == 'M') update_cache(cache[set_index], tag, records, E, verbose);
        if (verbose) printf("\n");
    }
}

void update_cache(cache_line_ptr lines, uint64_t tag, int* records, int E, int verbose) {
    int valid_line = -1, evict_line = 0, is_hit = 0;
    for (int i = 0; i < E; i++) {
        // hit
        if (lines[i].tag == tag && lines[i].valid) {
            records[0] += 1; // Update hit
            lines[i].counter = records[3]; // Update line counter
            is_hit = 1;
            if (verbose) printf("hit ");
            break;
        }
        if (valid_line < 0 && !lines[i].valid) valid_line = i; // Track empty line
        if (lines[i].valid && lines[i].counter < lines[evict_line].counter) evict_line = i; // Track LRU line
    }
    // miss && !eviction
    if (!is_hit && valid_line >= 0) {
        records[1] += 1; // Update miss
        lines[valid_line].tag = tag;
        lines[valid_line].counter = records[3];
        lines[valid_line].valid = 1;
        if (verbose) printf("miss ");
    }
    // miss && eviction
    if (!is_hit && valid_line < 0) {
        records[1] += 1; // Update miss
        records[2] += 1; // Update eviction
        lines[evict_line].tag = tag;
        lines[evict_line].counter = records[3];
        if (verbose) printf("miss eviction ");
    }
}
