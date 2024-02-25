#define DMALLOC_DISABLE 1
#include "dmalloc.hh"
#include <cassert>
#include <cstring>

#define FREE ((char*) 0xF7E3F7E3)
#define ALLOC ((char*) 0xA110C47E)
#define END ((char*)  0xE0D3E0D)

static struct dmalloc_stats global_stats = {0, 0, 0, 0, 0, 0, 0, 0};

typedef struct metadata {
    size_t data_sz; // size of user allocation
    char* data_ptr; // pointer to payload
    struct metadata* next;
    struct metadata* prev;
    char* state; 
    const char* file;
    long int line;
} metadata;

typedef struct end {
    char* state; 
} end;

metadata* head = NULL;

void insert_node(metadata* n) {
    n->next = head;
    n->prev = NULL;
    if (head != NULL) {
        head->prev = n;
    }
    head = n;
}

void remove_node(metadata* n) {
    if (n->next != NULL) {
        n->next->prev = n->prev;
    }
    if (n->prev != NULL) {
        n->prev->next = n->next;
    } else {
        head = n->next;
    }
}

/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of memory and 
 *      return a pointer to it 
 * 
 * @arg size_t sz : the amount of memory requested 
 * @arg const char *file : a string containing the filename from which dmalloc was called 
 * @arg long line : the line number from which dmalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings

    // check size overflow
    if (sz == (size_t) -1) {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }

    // allocate extra space
    void* block = base_malloc(sizeof(metadata) + sz + sizeof(end));

    // check if failed allocation
    if (block == NULL) {
        global_stats.nfail++;
        global_stats.fail_size += sz;
        return NULL;
    }

    // set metadata
    metadata* metadata_ptr = (metadata*) block;
    metadata_ptr->data_sz = sz;
    metadata_ptr->data_ptr = (char*)block + sizeof(metadata);
    metadata_ptr->state = ALLOC;
    metadata_ptr->file =  file;
    metadata_ptr->line = line;

    end* e = (end*) (metadata_ptr->data_ptr + metadata_ptr->data_sz);
    e->state = END;

    // update stats
    global_stats.ntotal++;
    global_stats.nactive++;
    global_stats.total_size += metadata_ptr->data_sz;
    global_stats.active_size += metadata_ptr->data_sz;

    insert_node(metadata_ptr);

    if (global_stats.heap_min == 0 || (uintptr_t) metadata_ptr->data_ptr <= (uintptr_t) global_stats.heap_min) {
        global_stats.heap_min = (uintptr_t) metadata_ptr->data_ptr;
    }

    if (global_stats.heap_max == 0 || (uintptr_t) metadata_ptr->data_ptr + sz >= (uintptr_t) global_stats.heap_max) {
        global_stats.heap_max = (uintptr_t) metadata_ptr->data_ptr + sz;
    }

    return metadata_ptr->data_ptr;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should 
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing. 
 * 
 * @arg void *ptr : a pointer to the heap 
 * @arg const char *file : a string containing the filename from which dfree was called 
 * @arg long line : the line number from which dfree was called 
 */
void dfree(void* ptr, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.

    if (ptr == NULL) {
        return;
    }

    if ((uintptr_t) ptr > global_stats.heap_max || (uintptr_t) ptr < global_stats.heap_min) {
       fprintf(stderr,"MEMORY BUG: %s:%li: invalid free of pointer %p, not in heap\n", file, line, ptr);
       abort();
    }

    metadata* metadata_ptr = (metadata*)((char*)ptr - sizeof(metadata));

    metadata* temp = head;
    int isALLOC = 0;
    while (temp != NULL) {
        if (temp->data_ptr == ptr) {
            isALLOC = 1;
            break;
        }
        temp = temp->next;
    } 

   // check for double frees
    if (metadata_ptr->state == FREE) {
        fprintf(stderr, "MEMORY BUG: %s:%li: invalid free of pointer %p, double free\n", file, line, ptr);
        abort();
    }

    if (!metadata_ptr || metadata_ptr->state != ALLOC || !isALLOC) {
        fprintf(stderr, "MEMORY BUG: %s:%li: invalid free of pointer %p, not allocated\n", file, line, ptr);
        metadata* t = head;
        while (t != NULL) {
            if ((t->data_ptr < ((char*) ptr)) && (((char*) ptr) < (t->data_ptr + t->data_sz))) {
                size_t n_bytes = (size_t) ptr - (size_t) t->data_ptr;
                fprintf(stderr, "%s:%li: %p is %zu bytes inside a %zu byte region allocated here\n", file, line-1, ptr, n_bytes, t->data_sz);
                abort();
            }
            t = t->next;
        }
        abort();
    }

    if (metadata_ptr->next && (metadata_ptr->next)->prev != metadata_ptr) {
       fprintf(stderr, "MEMORY BUG: %s:%li: %zu free of pointer %p\n", file, line, metadata_ptr->data_sz, ptr);
       abort();
    }

    end* e = (end*) ((char*) ptr + metadata_ptr->data_sz);
    if (e->state != END) {
        fprintf(stderr,"MEMORY BUG: %s:%li: detected wild write during free of pointer %p\n", file, line, ptr);
        abort();
    }

    // update stats
    global_stats.nactive--;
    global_stats.active_size -= metadata_ptr->data_sz;
    metadata_ptr->state = FREE;

    remove_node(metadata_ptr);
    base_free(metadata_ptr);
}

/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an array of `nmemb` 
 *      number of elements with wach element being `sz` bytes. The memory should be initialized 
 *      to zero  
 * 
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested for
 * @arg const char *file : a string containing the filename from which dcalloc was called 
 * @arg long line : the line number from which dcalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).
    size_t result_sz = nmemb*sz;
    void* ptr = NULL;
    
    // check size overflow
    if (result_sz < sz || result_sz < nmemb) {
		global_stats.nfail++;
		global_stats.fail_size += sz;
		return NULL;
	}

    ptr = dmalloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics  
 * 
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(dmalloc_stats));
    // Your code here.
    stats->nactive = global_stats.nactive;
    stats->active_size = global_stats.active_size;
    stats->ntotal = global_stats.ntotal;
    stats->total_size = global_stats.total_size;
    stats->nfail = global_stats.nfail;
    stats->fail_size = global_stats.fail_size;
    stats->heap_min = global_stats.heap_min;
    stats->heap_max = global_stats.heap_max;
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout       
 */
void print_statistics() {
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**  
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.
    if (head != NULL){
        metadata* temp = head;
        while (temp != NULL) {
            if (temp->state == ALLOC) {
		        printf("LEAK CHECK: %s:%li: allocated object %p with size %zu\n", temp->file, temp->line, temp->data_ptr,temp->data_sz);
            }
	        temp = temp->next;
        }
    }

}

/// drealloc(ptr, sz, file, line)
///    Reallocate the dynamic memory pointed to by `ptr` to hold at least
///    `sz` bytes, returning a pointer to the new block. If `ptr` is
///    `nullptr`, behaves like `dmalloc(sz, file, line)`. If `sz` is 0,
///    behaves like `dfree(ptr, file, line)`. The allocation request
///    was at location `file`:`line`.

void* drealloc(void* ptr, size_t sz, const char* file, long line) {
    metadata* new_ptr = NULL;
    if (sz) {
        new_ptr = (metadata*) dmalloc(sz, file, line);
    }
    if (ptr && new_ptr) {
        metadata *metadata_ptr = (metadata*)((char*)ptr - sizeof(metadata));
        size_t old_sz = metadata_ptr->data_sz;
        if (old_sz < sz) {
            memcpy(new_ptr, ptr, old_sz);
        } 
        else {
            memcpy(new_ptr, ptr, sz);
        }
    }
    dfree(ptr, file, line);
    return (void*) new_ptr;
}
