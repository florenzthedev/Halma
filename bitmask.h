#ifndef BITMASK_H_INCLUDED
#define BITMASK_H_INCLUDED
/* bitmask.h
 * Library for creating simple 2D bitmasks. Two dimensional arrays of single
 * bits. Useful for board games in memory-constrained enviroments.
 */
#include <limits.h>
#include <stdbool.h>

typedef unsigned short bitindex_T;
#define BITINDEX_T_MAX USHRT_MAX

/**
 * @brief Struct that holds bitmask and supporting data.
 * Avoid accessing if you can help it, this is supposed to be opaque.
 */
typedef struct {
  unsigned char* bits;
  bitindex_T size_y;
  bitindex_T size_x;
} bitmask_T;

/**
 * @brief Allocate and initialize a bitmask. Bitmask will be initialized to all
 * zeros/false.
 *
 * @param size_y Number of rows of bits to store.
 * @param size_x Number of columns of bits to store.
 * @return Bitmask, allocated on the heap.
 */
bitmask_T* create_bitmask(bitindex_T size_y, bitindex_T size_x);

/**
 * @brief Gets the value of a bit at a specified index.
 * This function does NOT perform bounds checking.
 * @param mask Mask to get bit from.
 * @param index_y Row bit is on.
 * @param index_x Column bit is on.
 * @return The value of the bit at [index_y][index_x].
 */
bool getbit(bitmask_T* mask, bitindex_T index_y, bitindex_T index_x);

/**
 * @brief Sets the value of a bit to 1 at a specified index.
 *
 * @param mask Mask to set bit on.
 * @param index_y Row bit is on.
 * @param index_x Column bit is on.
 */
void setbit(bitmask_T* mask, bitindex_T index_y, bitindex_T index_x);

/**
 * @brief Sets the value of a bit to 1 at a specified index and returns its
 * original value.
 * @param mask Mask to get and set the bit from.
 * @param index_y Row bit is on.
 * @param index_x Column bit is on.
 */
bool getsetbit(bitmask_T* mask, bitindex_T index_y, bitindex_T index_x);

/**
 * @brief Toggle the value of a bit at a specified index.
 * This function does NOT perform bounds checking.
 * @param mask Mask to set bit on.
 * @param index_y Row bit is on.
 * @param index_x Column bit is on.
 */
void togglebit(bitmask_T* mask, bitindex_T index_y, bitindex_T index_x);

/**
 * @brief AND's two masks together. The result is only stored in the result mask.
 * 
 * @param result bitmask to AND, this is the only one that gets modified
 * @param mask AND mask
 */
void andmask(bitmask_T* result, bitmask_T* mask);

/**
 * @brief Checks to see if any bit inside of the mask is true.
 *
 * @param mask Mask to check all bits of.
 * @return true if here is at least one bit that is true, false if there are no
 * bits that are true.
 */
bool anybit(bitmask_T* mask);

/**
 * @brief Resets entime bitmask back to zero.
 *
 * @param mask Mask to zero-out.
 */
void reset_bitmask(bitmask_T* mask);

/**
 * @brief Deallocates bitmask.
 *
 * @param mask Mask to deallocate.
 */
void destroy_bitmask(bitmask_T* mask);

#endif /* BITMASK_H_INCLUDED */