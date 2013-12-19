/*
  Copyright (C) 2013 Yordan Miladinov

  This file is part of Multithreaded mergesort.

  Multithreaded mergesort is free software: you can redistribute it
  and/or modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  Multithreaded mergesort is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Multithreaded mergesort.  If not, see
  <http://www.gnu.org/licenses/>.

  Yordan Miladinov <yordan@4web.bg>
*/

#include "mergesort.h"
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#define ELEMENT(arr, i, size) ((void *) ((char *) (arr) + ((size) * (i))))

#define COPY_ELEMENT(src, src_index, dest, dest_index, size) \
  memcpy (ELEMENT (dest, dest_index, size), \
          ELEMENT (src, src_index, size), size);

typedef struct
{
  size_t       size;            /* Size of each array element */
  int (*cmp) (const void *lhs, const void *rhs); /* Element comparator */
  sem_t        mutex;
  unsigned int nthreads;        /* Currently running threads */
  unsigned int K;               /* Maximal number of threads */
} mergesort_cfg_t;

typedef struct
{
  void            *base;
  size_t           len;
  mergesort_cfg_t *cfg;
} slice_t;

static slice_t *
create_slice (void *base, size_t len, mergesort_cfg_t *cfg);

/**
 * Merge arrays a and b in c.  Assume size_c >= size_a + size_b.
 */
static void
merge (void *arr1, size_t len1,
       void *arr2, size_t len2,
       size_t size, void *result,
       int (*cmp) (const void *lhs, const void *rhs));

/**
 * This is the core mergesort function.  The other one -- mergesort() --
 * is a simple wrapper that does a onetime initialization before start.
 */
static void
mergesort_core (void *base, size_t num, mergesort_cfg_t *cfg);

static void *
sort_slice (void *arg);

void
mergesort (void *base,
           size_t num,
           size_t size,
           int (*cmp) (const void *lhs, const void *rhs),
           const unsigned int K)
{
  mergesort_cfg_t *cfg;

  cfg = malloc (sizeof (mergesort_cfg_t));
  cfg->size = size;
  cfg->K = K;
  cfg->cmp = cmp;
  cfg->nthreads = 1;
  sem_init (&cfg->mutex, 0, 1);

  mergesort_core (base, num, cfg);
  free (cfg);
}

static slice_t *
create_slice (void *base, size_t len, mergesort_cfg_t *cfg)
{
  slice_t *ret;

  ret = malloc (sizeof (slice_t));
  ret->base = base;
  ret->len = len;
  ret->cfg = cfg;

  return ret;
}

static void
merge (void *a, size_t lena,
       void *b, size_t lenb,
       size_t size, void *out,
       int (*cmp) (const void *lhs, const void *rhs))
{
  size_t ia, ib, iout;

  ia = ib = iout = 0;
  while (ia < lena && ib < lenb)
    {
      if (cmp (ELEMENT (a, ia, size), ELEMENT (b, ib, size)) < 0)
        {
          COPY_ELEMENT (a, ia, out, iout, size);
          ia++;
          iout++;
        }
      else
        {
          COPY_ELEMENT (b, ib, out, iout, size);
          ib++;
          iout++;
        }
    }

  while (ia < lena)
    {
      COPY_ELEMENT (a, ia, out, iout, size);
      ia++;
      iout++;
    }

  while (ib < lenb)
    {
      COPY_ELEMENT (b, ib, out, iout, size);
      ib++;
      iout++;
    }
}

static void
mergesort_core (void *base, size_t num, mergesort_cfg_t *cfg)
{
  void    *arr1;                /* Slice 1 */
  void    *arr2;                /* Slice 2 */
  size_t   i;
  size_t   len1;                /* Size of slice 1 */
  size_t   len2;                /* Size of slice 2 */
  slice_t *slice1 = NULL;
  slice_t *slice2 = NULL;

#ifdef MERGESORT_PARALLEL
  bool                 is_thread_started = false;
  pthread_t            sort_slice_thread;
  int                  join;    /* pthread_join return coode */
#endif /* MERGESORT_PARALLEL */

#ifdef MERGESORT_DEBUG
  printf ("thread_id=%lu, enter mergesort (a, %u);\n", pthread_self (), num);
#endif

  /* Do not process arrays with length of 0 or 1 */
  if (num <= 1)
    return;

  /* Compute slice lengths */
  len1 = num / 2;
  len2 = num - len1;

  /* Alloc slices and initialize */
  arr1 = malloc (cfg->size * len1);
  for (i = 0; i < len1; i++)
    COPY_ELEMENT (base, i, arr1, i, cfg->size);

  arr2 = malloc (cfg->size * len2);
  for (i = len1; i < num; i++)
    COPY_ELEMENT (base, i, arr2, i-len1, cfg->size);

  /* Sort slices */

  /* Process slice 2 first, since slice 1 will be sorted always here, in
     this thread */

  /* Check for length as it's unnecessary to start a task (concurrent or
     not) for arrays with length <= 1 */
  if (len2 > 1)
    {
      slice2 = create_slice (arr2, len2, cfg);

#ifdef MERGESORT_PARALLEL
      sem_wait (&cfg->mutex);
      if (cfg->nthreads < cfg->K)
        {
          cfg->nthreads++;
          pthread_create (&sort_slice_thread, NULL, sort_slice, slice2);
          is_thread_started = true;

#ifdef MERGESORT_DEBUG
          printf ("thread_id=%lu, start new thread with id=%lu, nthreads=%d\n",
                  pthread_self(), sort_slice_thread, cfg->nthreads);
#endif /* MERGESORT_DEBUG */

        }
      sem_post (&cfg->mutex);

      if (is_thread_started == false)
        sort_slice (slice2);
#else  /* MERGESORT_PARALLEL is not defined */
      sort_slice (slice2);
#endif /* MERGESORT_PARALLEL */

    }

  /* Sort the first slice */
  if (len1 > 1)
    {
      slice1 = create_slice (arr1, len1, cfg);
      sort_slice (slice1);
    }

#ifdef MERGESORT_PARALLEL
  if (is_thread_started)
    {
      /* Wait for concurrent sorting to finish if it's not already
         done */
      join = pthread_join (sort_slice_thread, NULL);
      assert (join == 0);

      sem_wait (&cfg->mutex);
      cfg->nthreads--;
      sem_post (&cfg->mutex);

#ifdef MERGESORT_DEBUG
      printf ("thread_id=%lu, join thread with id=%lu, nthreads=%d\n",
              pthread_self (), sort_slice_thread, cfg->nthreads);
#endif /* MERGESORT_DEBUG */

    }
#endif /* MERGESORT_PARALLEL */

  /* merge slices */
  merge (arr1, len1, arr2, len2, cfg->size, base, cfg->cmp);

  /* free slices */
  if (slice1 != NULL)
    free (slice1);

  if (slice2 != NULL)
    free (slice2);

  free (arr1);
  free (arr2);

#ifdef MERGESORT_DEBUG
  printf ("thread_id=%lu, exit mergesort (a, %u);\n", pthread_self (), num);
#endif
}

static void *
sort_slice (void *arg)
{
  slice_t *slice = (slice_t *) arg;

#ifdef MERGESORT_DEBUG
  struct rusage usage;

  printf ("thread_id=%lu start : mergesort(array_length=%4d)\n",
    pthread_self (), slice->len);
#endif  /* MERGESORT_DEBUG */

  mergesort_core (slice->base, slice->len, slice->cfg);

#ifdef MERGESORT_DEBUG
  getrusage (1, &usage);
  printf ("thread_id=%lu finish: mergesort(array_length=%4d),"
          "usage.sec=%ld, usage.usec=%ld\n",
          pthread_self (), slice->len,
          usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
#endif  /* MERGESORT_DEBUG */

  return NULL;
}
