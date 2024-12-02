Caching I/O
======================

<!-- TODO: Fill this out. -->

## Design Overview: 
For the design overview, I've broken up the implementation into five main parts:

Initialization
Read
Write
Flush
Open/Close

### Initialization
First, I set up the metadata required for this caching project. Here's the relevant metadata used in my implementation:

'head': This tracks where we are in the file based on user operations (reads/writes). It increments as we perform operations and represents our current position in the file.
'cache_start': Tracks where our cache begins in the file. This gets updated whenever we load a new cache block.
'cache_end': Marks the end position of our current cache block in the file. This is used to determine when we need to fetch new data.
'dirty': Flag that's set to 0 if cache hasn't been modified, 1 if it has. Used in flush() to determine if we need to write back to the file.
'direction': Keeps track of access direction for optimizing prefetch behavior.
'file_size': Maintains the current size of the file, important for EOF detection and write operations.

### Read
This section is split into two parts: readc() and read():
'readc()'
Since we're only reading one byte at a time, implementation is straightforward
Checks if current position (head) is at 'cache_end'
If at 'cache_end', triggers 'fetch()' to load new cache block
Returns -1 on EOF or error, character value otherwise

'read()'

For bulk read operations, follows this process:

Checks for EOF condition ('head' >= 'file_size')
If at cache boundary, triggers 'fetch()' for new cache block
Handles cases where read size exceeds remaining cache:

Copies remaining cache data to buffer
Updates head and cache positions
Fetches new cache block if needed


For normal case (enough cache space), copies requested amount
Returns number of bytes read



### Write
This section is split into two parts: writec() and write():
'writec()'
Handles single byte writes to the cache
Sets dirty flag when modifying cache
If cache is full, triggers flush() and resets cache
Returns written character or -1 on error

'write()'
For bulk write operations:
If at cache boundary, flushes current cache
Handles cases where write size exceeds cache space:

Fills current cache
Flushes cache
Continues with remaining data


Updates 'cache_end' if writing beyond current cache boundary
Sets dirty flag for modified cache
Returns number of bytes written



### Flush
'flush()'

Implementation is straightforward:
- Checks dirty flag
- If dirty, writes cache contents to file using pwrite()
- Uses cache_start and head to determine write size
- Resets dirty flag after successful write
- Returns 0 on success, -1 on error



### Open/Close
'open()'
Two main goals:

Initialize metadata:
- Set head, cache_start, cache_end to 0
- Allocate cache buffer
- Initialize dirty flag and direction


Perform initial cache setup:
- Get file size
- Load initial cache block
- Set up initial boundaries

'close()'
Main considerations:
- Final flush if cache is dirty
- Free allocated cache buffer
- Close file descriptor
- Return appropriate status

Throughout the implementation, careful attention was paid to maintaining proper cache boundaries and minimizing system calls through the use of the dirty flag and optimized read/write operations.



