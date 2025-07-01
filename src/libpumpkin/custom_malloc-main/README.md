# custom_malloc
A simple implementation of malloc. A detailed description of the implementation
can be found at ![here](https://medium.com/@dheeptuck/how-to-implement-malloc-e2396c8c2376).

## Data Structures
1. **avail_dll**: One doubly linked list(DLL) to track available blocks. Each node in the DLL represents a block of memory and is called a block descriptor(BD).
2. **alloc_dll**: Another DLL to track allocated blocks. The DLL maintains the BD's in sorted order. The sorting is based on the address of the blocks pointed by the BD's. i.e the first BD in the DLL will point to the free block with the least address. 
The BD's are placed in the heap. Each BD is placed in the heap such that it immediately precedes the block it is pointing.

## Algorithm
1. **malloc**: The algorithm follows a first fit approach. malloc searches for the first apt free block. The identified free block could have more memory than requested. Hence, the block is split into two, one block allocated to the malloc caller and another unallocated block. The BD pointing to the allocated block is added to alloc_dll. A new BD pointing to the unallocated block is created and  added to avail_dll. The time complexity grows linearly based on the number of blocks.
2. **free**: Removes the BD pointing to the block to be freed from alloc_dll. The removed BD is added to avail_dll such that avail_dll remains sorted. Subsequently, a check is made to verify whether the immediately preceding BD in the avail_dll points to the immediately preceding block in the heap. If yes, both blocks are merged combine the blocks by combining BD's. Similarly, a check is made to verify whether the immediately succeeding BD corresponds to the immediately succeeding block in the heap. If yes, the blocks are merged by combining BD's. The time complexity grows linearly based on the number of free blocks.

**Files**
1. The implementation of the core is in custom_malloc.c.
2. The tests can be found in tests.c. Each function is a test.

## Building & Running
make clean  
make  
make run  
