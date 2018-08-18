
#include <time.h>
#include "curl_rand.h"


/* Private pseudo-random number seed. Unsigned integer >= 32bit. Threads
   mutual exclusion is not implemented to acess it since we do not require
   high quality random numbers (only used in form boudary generation). */

static unsigned int randseed;

/* Pseudo-random number support. */

unsigned int Curl_rand(void)
{
  unsigned int r;
  /* Return an unsigned 32-bit pseudo-random number. */
  r = randseed = randseed * 1103515245 + 12345;
  return (r << 16) | ((r >> 16) & 0xFFFF);
}

unsigned int  Curl_srand(void)
{
  /* Randomize pseudo-random number sequence. */

  randseed = (unsigned int) time(NULL);
  Curl_rand();
  Curl_rand();
  Curl_rand();
}

