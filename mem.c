#include "include.h"
#include "mem.h"



#ifdef WATCH_MEMORY


static int stats_alloc = 0, stats_freed = 0, stats_alloc_b = 0;



void* w_malloc (size_t s)
{
//	printf(" ~~~~~~~~~~~ allocating %d bytes ~~~~~~~~~~~~~\n", s);
	stats_alloc++;
	stats_alloc_b += s;
	
	return malloc(s);
}
void w_free (void* b)
{
	stats_freed++;
	free(b);
}
void memory_stats()
{
	printf("\n"
           "    =+=  Memory stats  =+=  \n"
	       " > Allocated: %d blocks (%d bytes)\n"
		   " > Freed:     %d blocks\n\n",
		stats_alloc, stats_alloc_b,
		stats_freed);
}






#endif

extern void* w_dup (void* d, size_t s)
{
	void* o = w_malloc(s);
	memcpy(o, d, s);
	return o;
}