#ifndef _MERGESORT_H_
#define _MERGESORT_H_

#include <stddef.h>

/* If this is defined, mergesort() will use multiple threads to
   sort. */
#define MERGESORT_PARALLEL 1

/* Debug logs */
/* #define MERGESORT_DEBUG 1 */

/**
 * Multithreaded mergesort implementation.
 *
 * @param base Pointer to the first object of the array to be sorted
 * @param num Number of elements in that array
 * @param size Size in bytes of array elements
 * @param cmp Pointer to a function that compares two array elements
 * @param K maximum number of simultaneous threads running at any moment
 */
void mergesort (void *base,
                size_t num,
                size_t size,
                int (*cmp) (const void *lhs, const void *rhs),
                const unsigned int K);

#endif /* _MERGESORT_H_ */
