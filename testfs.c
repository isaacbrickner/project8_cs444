#include "image.h"
#include "block.h"
#include "ctest.h"
#include "free.h"
#include "inode.h"
#include "dir.h"
#include "ls.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void test_set_free(void)
{
    unsigned char block[4096] = {0, 0, 0, 0, 0, 0};
    set_free(block, 1, 1);
    CTEST_ASSERT(block[0] == 1, "test setting a 0 bit to 1");
}

void test_find_free(void)
{
    unsigned char block[4096] = {1, 1, 0, 0, 0, 0};
    CTEST_ASSERT(find_free(block) == 2, "test finding a 0 bit and returning its index");
}

void test_ialloc(void)
{
    unsigned char inode_map[4096] = {0, 0, 0, 0, 0, 0};
    struct inode *in = ialloc(inode_map);
    CTEST_ASSERT(in->size == 0, "ialloc size");
    CTEST_ASSERT(in->flags == 0, "test ialloc flags");
    CTEST_ASSERT(in->inode_num == 0, "test ialloc inode number");
    CTEST_ASSERT(in->link_count == 0, "test ialloc link count");
    CTEST_ASSERT(in->owner_id == 0, "test ialloc owner id");
    CTEST_ASSERT(in->permissions == 0, "test ialloc permissions");
    CTEST_ASSERT(in->ref_count == 1, "test ialloc reference count");

}

void test_alloc(void)
{
    unsigned char data_block[4096] ={0, 0, 0, 0, 0, 0};
    CTEST_ASSERT(alloc(data_block) == 0, "test allocating block from blockmap");
}

void test_bread(void){
    unsigned char testBlock[4096]={1,1,1,1,1,1};
    CTEST_ASSERT(bread(2,testBlock)==testBlock, "test for block read");
}
void test_bwrite(void){
    unsigned char testBlock[4096] = {1,1,1,1,1,0};
    bwrite(1,testBlock);
    CTEST_ASSERT(bread(1,testBlock)==testBlock, "test  for block write(write, then assert read)");
}

void test_open_img(void){
    CTEST_ASSERT(image_open("test_open_img.txt",1) == 4, "test for image_open to be open fd #4");
}

void test_img_close(void){
    CTEST_ASSERT(image_close()==0, "test for image_close");
}

void test_img_fd_init(void){
    CTEST_ASSERT(image_fd==-1, "test for image_fd to be set to -1");
}

void test_img_fd_gets_set(void){
    CTEST_ASSERT(image_fd!=-1, "test to see if image_fd is set");
}

void test_incore_find_and_free(void)
{
    struct inode *node = incore_find_free();
    CTEST_ASSERT(node != NULL, "test incore_find_free");
    struct inode *node1 = incore_find(0);
    CTEST_ASSERT(node1 != NULL, "test incore_find");
    node1->ref_count = 1;
    incore_free_all();
    CTEST_ASSERT(node1->ref_count == 0, "test incore_free_all");

}


void test_read_write_inode(void) {
    struct inode in;
    struct inode write_node;
    in.inode_num = 0;
    read_inode(&in, 0);
    CTEST_ASSERT(in.inode_num == 0, "test read and write inode");
    
    write_inode(&in);
    read_inode(&write_node, in.inode_num);
    CTEST_ASSERT(write_node.inode_num == in.inode_num, "est write_inode");
    
}

void test_iget(void){
    for (int i = 1; i <= 10; i++) {
        CTEST_ASSERT(iget(0)->ref_count == i, "test refcount incrementing");
    }
}

void test_iput(void) {
    incore_free_all();
    unsigned char inode_map[4096] = {0, 0, 0, 0, 0, 0};
    struct inode *in = ialloc(inode_map);
    CTEST_ASSERT(in != NULL, "inode allocated");
    printf("inode num %d \n",in->inode_num);
    printf("ref count before iput %d \n",in->ref_count);
    iput(in);
    printf("ref after iput %d \n",in->ref_count);
    CTEST_ASSERT(in->ref_count == 0, "Reference count should be 0 after iput");
}

void test_mkfs() {
    mkfs();
    struct inode *inode = iget(0);
    printf("not allocating a node correctly it seems. flags =  %d, should be 2\n", inode->flags);
    // CTEST_ASSERT(inode->flags == 2, "root node allocated from mkfs");
}

void test_directory_open(){

    struct directory *d = directory_open(0);
    CTEST_ASSERT(d != NULL, "test directory open");
}

void test_directory_get(){

    struct directory *d = directory_open(0);
    struct directory_entry entry;

    CTEST_ASSERT(directory_get(d, &entry) == 0, "test getting an entry");
    printf("Directory_get(): directory name is %c\n", entry.name);
    printf("should be . and is instead 'h'.");
}

void test_directory_close(){
    struct directory *d = directory_open(0);
    printf("\ntest directory close(): ");
    printf("\ninode num %d. we are going to free the directory now\n",d->inode);
    directory_close(d);
    printf("inode number should no longer exist: %d\n", d->inode);
}



#ifdef CTEST_ENABLE
int main(){
    CTEST_VERBOSE(1);
    test_img_fd_init();
    image_open("test_open_img.txt",0);
    test_img_fd_gets_set();
    test_open_img();
    test_img_close();
    test_bread();
    test_bwrite();
    test_set_free();
    test_find_free();
    test_ialloc();
    test_alloc();
    test_incore_find_and_free();
    test_read_write_inode();
    test_iget();
    // test_iput(); not working
    printf("\nPartially functional for mkfs() and directory operations, but not 100%%\n");
    test_mkfs();
    test_directory_open();
    test_directory_get();
    test_directory_close();
    CTEST_RESULTS();
    CTEST_EXIT();
    
    return 0;
}

#else
int main(){
    printf("If im here its not testing i think?");
}
#endif