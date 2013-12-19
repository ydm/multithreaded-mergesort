/*
  Copyright (C) 2013 Yordan Miladinov

  This file is part of Multithreaded mergesort.

  Foobar is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Foobar is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

  Yordan Miladinov <yordan@4web.bg>
*/

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
