#include "free.h"

#define BYTE_SIZE 8

void set_free(unsigned char *block, int num, int set){
    // find index of the bit
    int byte_index = num / BYTE_SIZE;
    int bit_index = num % BYTE_SIZE;

    if (set == 1) { 
        bit_index = find_low_clear_bit(block[byte_index]);
        block[byte_index] |= (1 << bit_index); 
    }
    else {
        block[byte_index] &= ~(1 << bit_index); 
    }
}

int find_free(unsigned char *block)
{
    for (int i = 0; i < BYTE_SIZE; i++) {
        if (block[i] == 0) {
            return i;
        }
    }
    return -1;
}

int find_low_clear_bit(unsigned char x)
{
    for (int i = 0; i < 8; i++)
        if (!(x & (1 << i)))
            return i;

    return -1;
}

