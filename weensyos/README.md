# Design Overview

This document provides a conceptual design overview of the main functions implemented inside 'kernel.cc'. Below are descriptions of each major function and its implementation.

## kernel()

The 'kernel' function's primary responsibility is initializing our kernel page table. During initialization, several conditions are checked:

- **'CONSOLE_ADDR'**: During each iteration, we verify if the virtual address equals the 'CONSOLE_ADDR'. If true, all permissions for the console address are set.

- **'PROC_START_ADDR'**: For each iteration, we check if the virtual address is within the kernel range (0 to 'PROC_START_ADDR'). Virtual addresses in this range are not given user-accessible permissions to prevent users from accessing kernel information directly. This creates a protected boundary between user space and kernel space.

- **'NULL_POINTER'**: For each iteration, if the virtual address is not the null pointer, all permissions are set (user-space). If the virtual address equals the null pointer, it's initialized to 0.

## process_setup()

This function handles several critical setup operations:

1. Initially uses 'kalloc()' to allocate a new pagetable, then copies mappings from the kernel pagetable initialized in the previous step.

2. Uses 'loader()' for each memory segment to allocate and map the rest of the memory.

3. Concludes with stack allocation, setting the stack address to 'MEMSIZE_VIRTUAL - PAGESIZE'. This allows each process's stack to grow downward starting at that address.

## syscall_page_alloc()

This function manages heap allocation with the following key points:

- Returns -1 for invalid requested addresses
- Invalid cases include:
  - Address not divisible by 'PAGESIZE'
  - Address less than processor starting address
  - Address greater than or equal to 'MEMSIZE_VIRTUAL'
- For valid addresses, maps the physical page using 'kalloc()' to the requested virtual address

## syscall_fork()

This function handles the 'SYSCALL_FORK' system call with the following process:

1. Searches for a free process slot in the 'ptable[]' array (excluding slot 0, which is reserved for the kernel)
2. Upon finding a free slot, initializes 'pid' variable
3. Creates a copy of the current's pagetable for the child pagetable
4. Copies the mappings and data from parent's pagetable to child's pagetable
5. Fills in the respective 'proc' struct fields using the 'pid' variable
6. Returns 'pid' to conclude the function

Important: If any mapping or 'kalloc()' fails, ensures proper cleanup and returns -1 to prevent memory leaks.

## syscall_exit()

This function manages program termination, primarily implemented through a helper function 'sys_call_kfree_helper()':

- The helper function serves both 'syscall_exit()' and 'syscall_fork()'
- Reduces code redundancy by handling similar functionality in different parts of the codebase
- Implementation uses:
  - 'vmiter' to free pages in the page table
  - 'ptiter' to free the pages themselves after freeing pages inside the pagetable
  
The function concludes by changing the current process state to 'P_FREE' in the 'syscall_exit()' function.
