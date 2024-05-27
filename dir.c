#include "dir.h"
#include "block.h"
#include "inode.h"
#include "pack.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DATABLOCK_SIZE 4096
#define DIRECTORY_SIZE 64

void mkfs() {
    unsigned char inmap[DATABLOCK_SIZE] = {0} ;
    struct inode *root_node = ialloc(inmap);

    unsigned char data_block[DATABLOCK_SIZE] = {0};
    int block_number = alloc(data_block);

    unsigned char directory_block[DATABLOCK_SIZE] = {0};

    root_node->flags = 2;
    root_node->size = 2*DIRECTORY_SIZE;
    root_node->block_ptr[0] = block_number;
    root_node->inode_num = 0;

    // compute offsets for the . and .. entries. 
    write_u16(directory_block, root_node->inode_num);
    strcpy((char*)(directory_block + DIRECTORY_SIZE), ".");

    write_u16(directory_block + DIRECTORY_SIZE, root_node->inode_num);
    strcpy((char*)(directory_block + DIRECTORY_SIZE), "..");

    bwrite(block_number, directory_block);
    iput(root_node);

}


struct directory *directory_open(int inode_num){

    struct inode *in = iget(inode_num);

    if(in == NULL) { 
        return NULL;
    }

    // new space for directory
    struct directory *directory = malloc(sizeof(struct directory));
    directory->inode = in;
    directory->offset = 0;

    return directory;
}


int directory_get(struct directory *dir, struct directory_entry *ent){

    if (dir->offset >= dir->inode->size) {
        return -1;
    }

    int block_index = dir->offset / DATABLOCK_SIZE;
    int block_offset = dir->offset % DATABLOCK_SIZE;
    int block_number = dir->inode->block_ptr[block_index];

    unsigned char block[DATABLOCK_SIZE];

    bread(block_number, block);

    ent->inode_num = read_u16(block + block_offset);
    strcpy(ent->name, (char*)(block + block_offset + 2));
    dir->offset += DIRECTORY_SIZE;

    return 0;

}

void directory_close(struct directory *d) {

    iput(d->inode);
    free(d);
}

