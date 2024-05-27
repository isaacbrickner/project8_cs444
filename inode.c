#include "inode.h"
#include "free.h"
#include "block.h"
#include "image.h"
#include "pack.h"
#include <stdlib.h>

#define BLOCK_SIZE 4096
#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3

#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE);

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

// Read the free inode bitmap block from disk
// Find a free inode in the bitmap
// Mark it use in-use
// Write the free inode bitmap block to disk
struct inode *ialloc(unsigned char *inode_map)
{
    bread(1, inode_map);
    int i = find_free(inode_map);
    if (i == -1)
    {
        return NULL;
    }
    // get incore version of inode
    struct inode *in = iget(i);
    set_free(inode_map, i, 1);

    // set properties to 0 and block pointers to 0
     in->permissions = 0;
    in->flags = 0;
    in->size = 0;
    in->owner_id = 0;
   
    for (int i = 0; i < INODE_PTR_COUNT; i++){
        in->block_ptr[i] = 0;
    }
    in->inode_num = i;
    
    write_inode(in);
    bwrite(1, inode_map);
    return in;
}



struct inode *incore_find(unsigned int inode_num) {

       for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if ( incore[i].inode_num == inode_num && incore[i].ref_count != 0)
        {
            return &incore[i];
        }
    }
    return NULL;
}

void incore_free_all(void){
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 0;
    }
}


struct inode *incore_find_free(void) {

    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++){
        if (incore[i].ref_count == 0){
            return &incore[i];
        }
    }
return NULL;
}



// void read_inode(struct inode *in, int inode_num): This takes a pointer to an empty struct inode that you're going to read the data into. The inode_num is the number of the inode you wish to read from disk.

// You'll have to map that inode number to a block and offset, as per above.

// Then you'll read the data from disk into a block, and unpack it with the functions from pack.c. And you'll store the results in the struct inode * that was passed in.


// copied  over to see structure of inode for offsets
// struct inode {
//     unsigned int size;
//     unsigned short owner_id;
//     unsigned char permissions;
//     unsigned char flags;
//     unsigned char link_count;
//     unsigned short block_ptr[INODE_PTR_COUNT];

//     unsigned int ref_count;  // in-core only
//     unsigned int inode_num;  // in-core only
// };

void write_inode(struct inode *in){
    // get offsets
    int block_num = in->inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = in->inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;


    // map into inode
    unsigned char block[BLOCK_SIZE];
    bread(block_num, block);

    unsigned char *blockAddress = block + (block_offset_bytes * INODE_SIZE);
    write_u32(blockAddress, in->size);
    write_u16(blockAddress + 4, in->owner_id);
    write_u8(blockAddress + 6, in->permissions);
    write_u8(blockAddress + 7, in->flags);
    write_u8(blockAddress + 8, in->link_count);

    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(blockAddress + 9 + (i * 2), in->block_ptr[i]);
    }

    bwrite(block_num, block);
}

void read_inode(struct inode *in, int inode_num) {
    // get offsets 
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;

    // map into inode
    unsigned char block[BLOCK_SIZE];
    bread(block_num, block);
    unsigned char *blockAddress = block + (block_offset_bytes * INODE_SIZE);

    in->size = read_u32(blockAddress);
    in->owner_id = read_u16(blockAddress + 4);
    in->permissions = read_u8(blockAddress + 6);
    in->flags = read_u8(blockAddress + 7);
    in->link_count = read_u8(blockAddress + 8);


    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(blockAddress + 9 + (i * 2));
        }

    in->ref_count = 1;
    in->inode_num = inode_num;
}


// The algorithm is this:

// Search for the inode number in-core (find_incore())
// If found:
// Increment the ref_count
// Return the pointer
// Find a free in-core inode (find_incore_free())
// If none found:
// Return NULL
// Read the data from disk into it (read_inode())
// Set the inode's ref_count to 1
// Set the inode's inode_num to the inode number that was passed in
// Return the pointer to the inode

struct inode *iget(int inode_num) {

    struct inode *in = incore_find(inode_num);
    if (in != NULL) {
        in->ref_count++;
        return in;
    }

    in = incore_find_free();

    if (in == NULL) {
        return NULL;
    }

    read_inode(in, inode_num);
    in->ref_count = 1;
    in->inode_num = inode_num;
    return in;
}

struct inode *iput(struct inode *in) {

    if (in->ref_count == 0) {
        return NULL;
    }
    in->ref_count--;
    if (in->ref_count == 0){
        write_inode(in); }
    return in;
}