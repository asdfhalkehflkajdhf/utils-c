
#include <time.h>
#include "random.h"


/* Private pseudo-random number seed. Unsigned integer >= 32bit. Threads
   mutual exclusion is not implemented to acess it since we do not require
   high quality random numbers (only used in form boudary generation). */


/* Pseudo-random number support. */

unsigned int random_u32(unsigned int *randseed)
{
  unsigned int r;
  /* Return an unsigned 32-bit pseudo-random number. */
  r = (*randseed) = (*randseed) * 1103515245 + 12345;
  return (r << 16) | ((r >> 16) & 0xFFFF);
}

unsigned int  random_u32_init(unsigned int *randseed)
{
  /* Randomize pseudo-random number sequence. */

  *randseed = (unsigned int) time(NULL);
  random_u32(randseed);
  random_u32(randseed);
  return random_u32(randseed);
}

