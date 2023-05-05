#include "bitmask.h"

#include <limits.h>
#include <stdlib.h>

bitmask_T *create_bitmask(bitindex_T size_y, bitindex_T size_x) {
  // calculate number of blocks needed
  // integer division rounds down, the part after the + checks if there is a
  // remainder and adds one if there is
  unsigned long blocks =
      ((size_y * size_x) / CHAR_BIT) + !!((size_y * size_x) % CHAR_BIT);
  if (blocks > BITINDEX_T_MAX) return NULL;  // TOO BIG!
  // allocate memory and assign variables
  bitmask_T *mask = malloc(sizeof(bitmask_T));
  if (mask == NULL) exit(39);
  mask->bits =
      calloc(blocks, sizeof(unsigned char));  // sizeof char is always 1
  mask->size_y = size_y;
  mask->size_x = size_x;

  return mask;
}

bool getbit(bitmask_T *mask, bitindex_T index_y, bitindex_T index_x) {
  bitindex_T block = ((mask->size_x * index_y) + index_x) / CHAR_BIT;
  unsigned char byte = mask->bits[block];
  byte = byte >> ((mask->size_x * index_y) + index_x) % CHAR_BIT;
  byte &= 1;

  // its already only going to be 1 or 0
  return byte;
}

void setbit(bitmask_T *mask, bitindex_T index_y, bitindex_T index_x) {
  bitindex_T block =
      ((mask->size_x * index_y) + index_x) / CHAR_BIT;  // index in our array
  unsigned char byte = 1;
  byte = byte << ((mask->size_x * index_y) + index_x) % CHAR_BIT;
  mask->bits[block] |= byte;
}

/* If anyone is wondering the performance delta between using getsetbit vs
 * getbit then setbit, with an x86_64 target using -Os and -S GCC's assembler
 * output gave getbit and setbit 14 assembly instructions each while getsetbit
 * was 20 instructions. This doesn't count the instructions needed to set up
 * the function calls or the call frame macro things.
 */

bool getsetbit(bitmask_T *mask, bitindex_T index_y, bitindex_T index_x) {
  bitindex_T block =
      ((mask->size_x * index_y) + index_x) / CHAR_BIT;  // index in our array
  unsigned char byte = 1;
  byte = byte << ((mask->size_x * index_y) + index_x) % CHAR_BIT;
  // conversion is just to be extra safe
  bool ret = !!(mask->bits[block] & byte);
  mask->bits[block] |= byte;
  return ret;
}

void togglebit(bitmask_T *mask, bitindex_T index_y, bitindex_T index_x) {
  bitindex_T block =
      ((mask->size_x * index_y) + index_x) / CHAR_BIT;  // index in our array
  unsigned char byte = 1;
  byte = byte << ((mask->size_x * index_y) + index_x) % CHAR_BIT;
  mask->bits[block] ^= byte;
}

bool anybit(bitmask_T *mask) {
  bitindex_T blocks = ((mask->size_y * mask->size_x) / CHAR_BIT) +
                      !!((mask->size_y * mask->size_x) % CHAR_BIT);
  for (bitindex_T i = 0; i < blocks; i++)
    if (mask->bits[i]) return true;
  return false;
}

void reset_bitmask(bitmask_T *mask) {
  bitindex_T blocks = ((mask->size_y * mask->size_x) / CHAR_BIT) +
                      !!((mask->size_y * mask->size_x) % CHAR_BIT);
  for (bitindex_T i = 0; i < blocks; i++) mask->bits[i] = 0;
}

void destroy_bitmask(bitmask_T *mask) {
  free(mask->bits);
  free(mask);
}
