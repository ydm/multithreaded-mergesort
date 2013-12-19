#include "mergesort.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#define USAGE1 "Usage: %s <in> <out> [K]                                \n\
    where:                                                              \n\
      in  -- Input file.  First line of the input file should contain   \n\
             the number of lines that follow.  Since this program is    \n\
             made just to show how the multithreaded mergesort function \n\
             is utilized, it's relatively simple and expects this"
#define USAGE2 "\n\
             helpful piece of meta information.  I apologize.  Oh!      \n\
             Another thing!  ;)  Each line should contain just a single \n\
             number (both integer and floating point numbers are fine). \n\
      out -- Output file                                                \n\
      K   -- Maximum number of simultaneous threads (defaults to 10)    \n"

int
cmp (const void *lhs, const void *rhs)
{
  double left = *((double *) lhs);
  double right = *((double *) rhs);
  double diff = left - right;

  if (diff < 0)
    return -1;
  else if (diff > 0)
    return 1;
  else
    return 0;
}

int
main (int argc, char **argv)
{
  double *a;
  size_t i;
  size_t n;
  FILE *in;
  FILE *out;
  int K;

#ifdef MERGESORT_DEBUG
  struct rusage usage;
#endif

  if (argc < 3)
    {
      printf (USAGE1, argv[0]);
      printf (USAGE2);
      exit (EXIT_FAILURE);
    }

  in = fopen (argv[1], "r");
  out = fopen (argv[2], "w");

  if (argc > 3)
    K = (int) strtol (argv[3], NULL, 10);
  else
    K = 10;

  if (in == NULL || out == NULL)
    {
      perror ("");
      exit (EXIT_FAILURE);
    }

  if (K <= 0)
    {
      printf ("K should be > 0\n");
      exit (EXIT_SUCCESS);
    }

  /* Read array */
  fscanf (in, "%u", &n);
  a = malloc (sizeof (double) * n);
  for (i = 0; i < n; i++)
    fscanf (in, "%lf", a + i);
  fclose (in);

  /* Sort it */
  mergesort (a, n, sizeof (double), cmp, K);

  /* Write array to file */
  for (i = 0; i < n; i++)
    fprintf (out, "%f\n", a[i]);
  fclose (out);

  /* Free array */
  free (a);

#ifdef MERGESORT_DEBUG
  getrusage (1, &usage);
  printf ("main thread finish: usage.sec=%ld, usage_usec=%ld\n",
          usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
#endif /* MERGESORT_DEBUG */

  return EXIT_SUCCESS;
}
