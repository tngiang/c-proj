#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../io300.h"

/*
    student.c
    Fill in the following stencils
*/

/*
    When starting, you might want to change this for testing on small files.
*/
#ifndef CACHE_SIZE
#define CACHE_SIZE 4096
#endif

#if (CACHE_SIZE < 4)
#error "internal cache size should not be below 4."
#error "if you changed this during testing, that is fine."
#error "when handing in, make sure it is reset to the provided value"
#error "if this is not done, the autograder will not run"
#endif

/*
   This macro enables/disables the dbg() function. Use it to silence your
   debugging info.
   Use the dbg() function instead of printf debugging if you don't want to
   hunt down 30 printfs when you want to hand in
*/
#define DEBUG_PRINT 0
#define DEBUG_STATISTICS 1

struct io300_file {
    /* read,write,seek all take a file descriptor as a parameter */
    int fd;
    /* this will serve as our cache */
    char* cache;

    // TODO: Your properties go here
    int dirty; // wether or not we have written to the cache
    off_t head; // where the read/write head is located in the file from the operations the user asked for (not including our prefetches)
    off_t cache_start; // where in the file our cache starts
    off_t cache_end; // where in the file our cache ends
    off_t file_size; // size of the file

    
    /* Used for debugging, keep track of which io300_file is which */
    char* description;
    /* To tell if we are getting the performance we are expecting */
    struct io300_statistics {
        int read_calls;
        int write_calls;
        int seeks;
    } stats;
};

/*
    Assert the properties that you would like your file to have at all times.
    Call this function frequently (like at the beginning of each function) to
    catch logical errors early on in development.
*/
static void check_invariants(struct io300_file* f) {
    assert(f != NULL);
    assert(f->cache != NULL);
    assert(f->fd >= 0);

    // TODO: Add more invariants
    assert(f->cache_start <= f->head && f->head <= f-> cache_end); 
    assert(f->cache_end - f->cache_start <= CACHE_SIZE); 
}

/*
    Wrapper around printf that provides information about the
    given file. You can silence this function with the DEBUG_PRINT macro.
*/
static void dbg(struct io300_file* f, char* fmt, ...) {
    (void)f;
    (void)fmt;
#if (DEBUG_PRINT == 1)
    static char buff[300];
    size_t const size = sizeof(buff);
    int n = snprintf(buff, size,
                     // TODO: Add the fields you want to print when debugging
                     "{desc:%s, } -- ", f->description);
    int const bytes_left = size - n;
    va_list args;
    va_start(args, fmt);
    vsnprintf(&buff[n], bytes_left, fmt, args);
    va_end(args);
    printf("%s", buff);
#endif
}

int io300_fetch(struct io300_file* const f);

struct io300_file* io300_open(const char* const path, char* description) {
    if (path == NULL) {
        fprintf(stderr, "error: null file path\n");
        return NULL;
    }

    int const fd = open(path, O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "error: could not open file: `%s`: %s\n", path,
                strerror(errno));
        return NULL;
    }

    struct io300_file* const ret = malloc(sizeof(*ret));
    if (ret == NULL) {
        fprintf(stderr, "error: could not allocate io300_file\n");
        close(fd);
        return NULL;
    }

    ret->fd = fd;
    ret->cache = malloc(CACHE_SIZE);
    if (ret->cache == NULL) {
        fprintf(stderr, "error: could not allocate file cache\n");
        close(ret->fd);
        free(ret);
        return NULL;
    }
    ret->description = description;
    // TODO: Initialize your file
    ret->cache_start = ret->head = 0;
    ret->cache_end = 0;
    ret->dirty = 0;
    ret->file_size = io300_filesize(ret);
    io300_fetch(ret);

    check_invariants(ret);
    dbg(ret, "Just finished initializing file from path: %s\n", path);
    return ret;
}

int io300_seek(struct io300_file* const f, off_t const pos) {
    check_invariants(f);
    f->stats.seeks++;
    if (f->dirty) {
        io300_flush(f);
    }
    off_t new_pos = lseek(f->fd, pos, SEEK_SET);
    if (new_pos == -1) {
        return -1;
    }
    f->head = pos;
    f->cache_start = f->head;
    f->cache_end = f->head;

    if (f->head > f->file_size) {
        f->file_size = f->head; // seek beyond eof
    } else {
        size_t to_read = (f->head + CACHE_SIZE) > f->file_size ? f->file_size - f->head : CACHE_SIZE;
        ssize_t n_read = read(f->fd, f->cache, to_read);
        if (n_read >= 0) {
            f->cache_end = f->head + n_read;
        } else {
            f->cache_end = f->head; // failed to read from the file
        }
    }

    return f->head;
}


int io300_close(struct io300_file* const f) {
    check_invariants(f);

#if (DEBUG_STATISTICS == 1)
    printf("stats: {desc: %s, read_calls: %d, write_calls: %d, seeks: %d}\n",
           f->description, f->stats.read_calls, f->stats.write_calls,
           f->stats.seeks);
#endif
    // TODO: Implement this
    if (f->dirty != 0) {
        io300_flush(f);
    }

    int ret = close(f->fd);
    free(f->cache);
    free(f);
    return ret;
}

off_t io300_filesize(struct io300_file* const f) {
    check_invariants(f);
    struct stat s;
    int const r = fstat(f->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode)) {
        return s.st_size;
    } else {
        return -1;
    }
}

int io300_readc(struct io300_file* const f) {
    check_invariants(f);
    // TODO: Implement this
    if (f->head > f->file_size) {
        return -1;
    }
    if(f->head == f->cache_end) {
        if (io300_fetch(f) == -1) { // if cache is empty, try to refill
            return -1;
        }
        if(f->head == f->cache_end) {
            return -1; // if cache still empty after trying to refill
        }
    }
    unsigned char c = f->cache[f->head -f->cache_start];
    f->head++;
    return (int) c;
}

int io300_writec(struct io300_file* f, int ch) {
    check_invariants(f);
    // TODO: Implement this
    if(f->head == f->cache_end) {
        if (f->head < io300_filesize(f)) {
            io300_fetch(f);
        } else if (io300_flush(f) == -1) {
            return -1;
        }
    }
    f->cache[f->head - f->cache_start] = ch;
    f->head++;
    f->dirty = 1;

    return ch;
}


ssize_t io300_read(struct io300_file* const f, char* const buff,
                   size_t const sz) {
    check_invariants(f);
    // TODO: Implement this

    if (f->head > f->file_size) {
        return 0;
    }
    size_t pos = 0;
    while (pos < sz) {
        if (f->head == f->cache_end) {
            io300_fetch(f);
            if (f->head == f->cache_end) {
                break; // if cache is still empty after fetching, break loop
            }
        }
        size_t n_to_copy = f->cache_end - f->head;
        if (n_to_copy > sz - pos) {
            n_to_copy = sz - pos;
        }
        memcpy(buff + pos, f->cache + (f->head - f->cache_start), n_to_copy);  
        f->head = f->head + n_to_copy;
        pos = pos + n_to_copy;
    }
    return pos; 
}


ssize_t io300_write(struct io300_file* const f, const char* buff,
                    size_t const sz) {
    check_invariants(f);
    // TODO: Implement this
    size_t pos = 0;
    while (pos < sz) {
        if (f->head == f->cache_start + CACHE_SIZE) {
            if (f->head < f->file_size) {
                io300_fetch(f);
            } else {
                if (f->cache_end < f->head) {
                    f->cache_end = f->cache_start + CACHE_SIZE;
                }
                if (io300_flush(f) == -1) {
                    return -1;
                }
            }
        }
        size_t remaining = sz - pos;
        size_t available = CACHE_SIZE - (f->head - f->cache_start);
        size_t n_to_write;

        if (remaining < available) {
            n_to_write = remaining;
        } else {
            n_to_write = available;
        }
        memcpy(f->cache + (f->head - f->cache_start), buff + pos, n_to_write);
        f->head = f->head + n_to_write;
        f->dirty = 1;
        pos = pos + n_to_write;
    }
    if (f->cache_end < f->head) {
        f->cache_end = f->cache_start + CACHE_SIZE;
    }
    return pos;
}

int io300_flush(struct io300_file* const f) {
    check_invariants(f);
    // TODO: Implement this
    lseek(f->fd, f->cache_start, SEEK_SET);
    ssize_t n_written = write(f->fd, f->cache, f->head - f->cache_start);
    if (n_written >= 0) {
        f->cache_start = f->head; // update to current head posn
        f->cache_end = f->cache_end + CACHE_SIZE;
        f->dirty = 0;
        return 0;
    }
    return -1;
}

int io300_fetch(struct io300_file* const f) {
    check_invariants(f);
    // TODO: Implement this
    /* This helper should contain the logic for fetching data from the file into the cache. */
    /* Think about how you can use this helper to refactor out some of the logic in your read, write, and seek functions! */
    /* Feel free to add arguments if needed. */
    if (f->dirty) {
        io300_flush(f);
    } else {
        f->cache_start = f->head = f->cache_end;
    }
    ssize_t n_read = read(f->fd, f->cache, CACHE_SIZE);
    if (n_read >= 0) {
        f->cache_end = f->head + n_read;
        return 0;
    }
    return -1;
}
