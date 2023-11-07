/*
All the main functions with respect to the MeMS are implemented here
read the function description for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITYw
*/
// add other headers as required
#include <stdio.h>
#include <stdlib.h>
/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this
macro to make the output of all system same and conduct a fair evaluation.
*/
#define PAGE_SIZE 4096

#include <sys/mman.h>
// implementation of a custom doubly linked list
struct node
{
    void *allocatedAddressPtr; // Pointer to the allocated memory
    _Bool isSubnode;           // Is this a sub-segment of a larger segment
    struct node *subChainPtr;  // Sub-chain of segments
    long size;                 // Size of the segment
    struct node *previousPtr;  // Pointer to the previous segment
    struct node *nextPtr;      // Pointer to the next segment
    _Bool isHole;              // Is this segment a hole in memory
    _Bool deleted;             // Is this segment deleted and available for reuse
};

int sizeNode = sizeof(struct node);
#define PAGE_SIZE 4096

struct node *returnNewAddr(int *currIndex, int *totalNodes, int *pageMultiple, struct node **startAddressPtr)
{
    // Function to return a new address for managing memory.
    // If there is a deleted node, reuse it.

    for (int i = 0; i < *currIndex; i++)
    {
        if (((*startAddressPtr) + i)->deleted)
        {
            return (*(startAddressPtr) + i);
        }
    }

    // If the current index is within the total nodes, increment it and return a new address.
    if (*currIndex < *totalNodes)
    {
        *currIndex += 1;
        return *startAddressPtr + *currIndex;
    }
    else
    {
        // If the current index exceeds the total nodes, allocate a new page.
        *pageMultiple += 1;
        struct node *newStartAddr = (struct node *)mmap(NULL, ((*pageMultiple) * PAGE_SIZE), PROT_READ | PROT_WRITE, 0x20 | MAP_PRIVATE, -1, 0);
        if (newStartAddr == MAP_FAILED)
        {
            perror("Fatal error: Failed to allocate memory.");
            exit(-1);
        }

        if (*pageMultiple > 1)
        {

            for (int i = 0; i < *currIndex; i++)
            {
                // Copy data from the old page to the new page if it exists.
                (newStartAddr + i)->subChainPtr = ((*startAddressPtr) + i)->subChainPtr;
                (newStartAddr + i)->size = ((*startAddressPtr) + i)->size;
                (newStartAddr + i)->previousPtr = ((*startAddressPtr) + i)->previousPtr;
                (newStartAddr + i)->nextPtr = ((*startAddressPtr) + i)->nextPtr;
                (newStartAddr + i)->isHole = ((*startAddressPtr) + i)->isHole;
                (newStartAddr + i)->deleted = ((*startAddressPtr) + i)->deleted;
                (newStartAddr + i)->subChainPtr = ((*startAddressPtr) + i)->subChainPtr;
                (newStartAddr + i)->isSubnode = ((*startAddressPtr) + i)->isSubnode;
            }

            if (munmap(*startAddressPtr, *pageMultiple * PAGE_SIZE) == -1)
            {
                // Unmap the old page.
                perror("Fatal error: Failed to de-allocate memory.");
                exit(-1);
            }
        }

        *totalNodes = ((*pageMultiple) * PAGE_SIZE) / sizeNode;
        *startAddressPtr = newStartAddr;

        return *startAddressPtr + *currIndex;
    }
}

void deleteNode(struct node *nodeAddressPtr)
{
    // Function to mark a node as deleted and available for reuse.

    nodeAddressPtr->deleted = 1;
}
struct node *headPtr;
int currIndex;
int totalNodes;
int pageMultiple;

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
// Function to initialize MeMS system parameters

void mems_init()
{
    currIndex = 0;
    totalNodes = 0;
    pageMultiple = 0;
    returnNewAddr(&currIndex, &totalNodes, &pageMultiple, &headPtr);

    headPtr->size = 0;
    headPtr->nextPtr = NULL;
    headPtr->previousPtr = NULL;
    headPtr->allocatedAddressPtr = NULL;
    headPtr->subChainPtr = NULL;
    headPtr->isHole = 0;
    headPtr->isSubnode = 0;
    headPtr->deleted = 0;
}
/*
Allocates memory of the specified size by reusing a segment from the free list if
a sufficiently large segment is available.

Else, uses the mmap system call to allocate more memory on the heap and updates
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/

void *mems_malloc(size_t reqSize)
{
    // Initialize the head iterator and the total size variable
    struct node *headIterPtr = headPtr;
    unsigned long long totalSize = 0;

    // Iterate over the main chain
    while (headIterPtr != NULL)
    {
        // Iterate over the sub-chain of segments
        struct node *subNodePtr = headIterPtr->subChainPtr;
        while (subNodePtr != NULL)
        {
            // Check if the segment is a hole and its size is greater than or equal to the requested size
            if (subNodePtr->isHole && ((subNodePtr->size) >= reqSize))
            {
                if ((subNodePtr->size) - reqSize > 0)
                {
                    // If the hole is larger than the requested size, split it
                    subNodePtr->isHole = 0;

                    // Create a new sub-segment with the remaining size
                    struct node *newSubNodePtr = returnNewAddr(&currIndex, &totalNodes, &pageMultiple, &headPtr);
                    newSubNodePtr->isHole = 1;
                    newSubNodePtr->deleted = 0;
                    newSubNodePtr->nextPtr = subNodePtr->nextPtr;
                    newSubNodePtr->previousPtr = subNodePtr;
                    newSubNodePtr->subChainPtr = NULL;
                    newSubNodePtr->size = (subNodePtr->size) - reqSize;
                    newSubNodePtr->allocatedAddressPtr = (subNodePtr->allocatedAddressPtr) + reqSize;

                    subNodePtr->nextPtr = newSubNodePtr;
                    subNodePtr->size = reqSize;
                    subNodePtr->isHole = 0;

                    if (newSubNodePtr->nextPtr != NULL)
                    {
                        newSubNodePtr->nextPtr->previousPtr = newSubNodePtr;
                    }

                    // Return the allocated memory address
                    return (void *)totalSize;
                }
                else
                {
                    // If the hole size matches the requested size, use the entire hole
                    subNodePtr->isHole = 0;

                    // Return the allocated memory address
                    return (void *)totalSize;
                }
            }
            totalSize += subNodePtr->size;
            subNodePtr = subNodePtr->nextPtr;
        }
        headIterPtr = headIterPtr->nextPtr;
    }

    // If no suitable hole is found, allocate a new page
    headIterPtr = headPtr;
    totalSize = 0;
    while (headIterPtr->nextPtr != NULL)
    {
        headIterPtr = headIterPtr->nextPtr;
        totalSize += headIterPtr->size;
    }

    // Create a new node for the main chain
    struct node *newNodePtr = returnNewAddr(&currIndex, &totalNodes, &pageMultiple, &headPtr);
    headIterPtr->nextPtr = newNodePtr;
    newNodePtr->isHole = 0;
    newNodePtr->deleted = 0;
    newNodePtr->isSubnode = 0;
    newNodePtr->nextPtr = NULL;
    newNodePtr->previousPtr = headIterPtr;
    newNodePtr->subChainPtr = returnNewAddr(&currIndex, &totalNodes, &pageMultiple, &headPtr);

    int multipleCalc;

    // Calculate the number of pages required to satisfy the requested size
    if (reqSize % PAGE_SIZE > 0)
    {
        multipleCalc = (reqSize / PAGE_SIZE) + 1;
    }
    else
    {
        multipleCalc = reqSize / PAGE_SIZE;
    }

    // Allocate memory for the new node
    newNodePtr->allocatedAddressPtr = (void *)mmap(NULL, multipleCalc * PAGE_SIZE, PROT_READ | PROT_WRITE, 0x20 | MAP_PRIVATE, -1, 0);
    newNodePtr->size = multipleCalc * PAGE_SIZE;

    // Create a sub-segment to represent the allocated memory
    struct node *subListNodePtr1 = newNodePtr->subChainPtr;
    subListNodePtr1->isHole = 0;
    subListNodePtr1->deleted = 0;
    subListNodePtr1->isSubnode = 1;
    subListNodePtr1->nextPtr = NULL;
    subListNodePtr1->previousPtr = NULL;
    subListNodePtr1->subChainPtr = NULL;
    subListNodePtr1->allocatedAddressPtr = newNodePtr->allocatedAddressPtr;
    subListNodePtr1->size = reqSize;

    // If there is unused space, create a new sub-segment to represent it
    if ((newNodePtr->size) > reqSize)
    {
        subListNodePtr1->nextPtr = returnNewAddr(&currIndex, &totalNodes, &pageMultiple, &headPtr);
        struct node *subListNodePtr2 = subListNodePtr1->nextPtr;

        subListNodePtr2->isHole = 1;
        subListNodePtr2->deleted = 0;
        subListNodePtr2->isSubnode = 1;
        subListNodePtr2->nextPtr = NULL;
        subListNodePtr2->previousPtr = subListNodePtr1;
        subListNodePtr2->subChainPtr = NULL;
        subListNodePtr2->allocatedAddressPtr = subListNodePtr1->allocatedAddressPtr + reqSize;
        subListNodePtr2->size = newNodePtr->size - subListNodePtr1->size;
    }

    // Return the allocated memory address
    return (void *)(totalSize);
}

/*
This function will be called at the end of the MeMS system and its main job is to unmap the
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/

void mems_finish()
{

    struct node *headIterPtr = headPtr->nextPtr;
    while (headIterPtr != NULL)
    {
        if (munmap(headIterPtr->allocatedAddressPtr, headIterPtr->size) == -1)
        {
            perror("Fatal error: Failed to de-allocate memory.");
            exit(-1);
        }
        headIterPtr = headIterPtr->nextPtr;
    }

    if (munmap(headPtr, pageMultiple * PAGE_SIZE) == -1)
    {
        perror("Fatal error: Failed to de-allocate memory.");
        exit(-1);
    }

    headPtr = NULL;
}

/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats()
{
    // all problems here only

    long totalAllocatedSize = 0;
    long totalFreeSize = 0;
    int ii = 0;
    int countPb = 0, a = 0;

    // implement counter that adds number of pages
    int subchain[1000];

    struct node *headIterPtr = headPtr->nextPtr;
    printf("----- MeMS SYSTEM STATS ----\n");
    long int apj = 0;
    // to keep track of page numbers, i.e 1000-1999 then to 2000
    while (headIterPtr != NULL)
    {
        int jj = 0;
        // to count pages

        totalAllocatedSize += headIterPtr->size;

        printf("MAIN[%ld:%ld]-> ", totalAllocatedSize - headIterPtr->size, totalAllocatedSize - 1);
        ii++;
        // fix this
        // think here along static printing lines

        struct node *subNodePtr = headIterPtr->subChainPtr;

        while (subNodePtr != NULL)
        {
            jj++;

            if (subNodePtr->isHole)
            {
                totalFreeSize += subNodePtr->size;
            }

            if (subNodePtr->isHole)
            {
                printf("P[%ld:%ld]", apj, apj + subNodePtr->size - 1);
                apj += subNodePtr->size;
                // think along printing here toox
            }
            else
            {
                printf("P[%ld:%ld]", apj, apj + subNodePtr->size - 1);
                /// fix this
                // maybe add new variable set to 0 to prevent same value form repeating itself

                apj += subNodePtr->size;
            }

            if (subNodePtr->nextPtr != NULL)
            {

                printf("<->");
            }

            subNodePtr = subNodePtr->nextPtr;
        }
        subchain[ii] = jj;

        if (headIterPtr->nextPtr != NULL)
        {
            printf("\n");
        }
        else
        {
            printf("-> NULL\n");

            printf("\n");
        }

        headIterPtr = headIterPtr->nextPtr;
    }

    printf("Pages Used : %ld\n", totalAllocatedSize / PAGE_SIZE);
    printf("Space unused : %li\n", totalFreeSize);
    printf("Main Chain Length: %d\n", ii);

    printf("Sub-Chain Length Array: [");

    // i is itert ative for loop variable and ii is number of times the
    // stat rows have been printed
    for (int i = 1; i < ii + 1; i++)
    {
        printf("%d ,", subchain[i]);
    }
    printf("]\n");
    printf("\n");
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void *vPtr)
{
    if (vPtr < 0)
    {
        fputs("Invalid virtual pointer\n", stdout);
        return (void *)-1;
    }

    // Initialize the head iterator
    struct node *headIterPtr = headPtr->nextPtr;

    while (headIterPtr != NULL)
    {
        if (vPtr < (void *)((long)headIterPtr->size))
        {
            // Iterate over the sub-chain of segments
            struct node *subNodePtr = headIterPtr->subChainPtr;

            while (subNodePtr != NULL)
            {
                if (vPtr < (void *)((long)subNodePtr->size))
                {
                    // Check if the sub-segment is not a hole (allocated)
                    if (!(subNodePtr->isHole))
                    {
                        // Return the physical address by adding the offset to the allocated address
                        return (subNodePtr->allocatedAddressPtr) + (long)(vPtr);
                    }
                    else
                    {
                        fputs("Invalid virtual pointer\n", stdout);
                        return (void *)-1;
                    }
                }
                vPtr -= subNodePtr->size;
                subNodePtr = subNodePtr->nextPtr;
            }
        }
        else
        {
            vPtr -= headIterPtr->size;
            headIterPtr = headIterPtr->nextPtr;
        }
    }

    fputs("Invalid: Virtual Pointer\n", stdout);
    return (void *)-1;
}

/*
mergeCo function is responsible for merging
ontiguous holes in the sub-chain
*/
void mergeCo()
{
    struct node *headIterPtr = headPtr->nextPtr;

    while (headIterPtr != NULL)
    {
        struct node *subNodePtr = headIterPtr->subChainPtr;

        while (subNodePtr->nextPtr != NULL)
        {
            // Check if the current sub-segment and the next one are both holes
            if (subNodePtr->isHole && subNodePtr->nextPtr->isHole)
            {
                struct node *nextSubNodePtr = subNodePtr->nextPtr;

                // Merge the holes by updating the size, address, and previous pointers
                nextSubNodePtr->size += subNodePtr->size;
                nextSubNodePtr->allocatedAddressPtr = subNodePtr->allocatedAddressPtr;
                nextSubNodePtr->previousPtr = subNodePtr->previousPtr;

                // Update the previous node's next pointer if it exists
                if (nextSubNodePtr->previousPtr != NULL)
                    nextSubNodePtr->previousPtr->nextPtr = nextSubNodePtr;

                subNodePtr->deleted = 1; // Mark the current sub-segment as deleted

                subNodePtr = nextSubNodePtr; // Move to the next sub-segment
            }
            else
            {
                subNodePtr = subNodePtr->nextPtr;
            }
        }
        headIterPtr = headIterPtr->nextPtr;
    }
}

/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing
*/

void mems_free(void *vPtr)
{
    if (vPtr < 0)
    {
        fputs("Invalid: Virtual Pointer\n", stdout);
        return;
    }
    struct node *headIterPtr = headPtr->nextPtr;
    while (headIterPtr != NULL)
    {
        if ((vPtr < (void *)((long)headIterPtr->size)))
        {
            struct node *subNodePtr = headIterPtr->subChainPtr;
            while (subNodePtr != NULL)
            {

                if (vPtr < (void *)((long)subNodePtr->size))
                {
                    if (!(subNodePtr->isHole))
                    {
                        subNodePtr->isHole = 1;

                        mergeCo();

                        return;
                    }
                    else
                    {
                        fputs("Invalid: Virtual Pointer\n", stdout);
                        return;
                    }
                }
                vPtr -= subNodePtr->size;
                subNodePtr = subNodePtr->nextPtr;
            }
        }
        else
        {
            vPtr -= headIterPtr->size;
            headIterPtr = headIterPtr->nextPtr;
        }
    }
    fputs("Invalid: Virtual Pointer\n", stdout);
    return;
}
