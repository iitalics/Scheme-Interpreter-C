#pragma once




//#define WATCH_MEMORY






#ifdef WATCH_MEMORY
extern void* w_malloc (size_t);
extern void w_free (void*);
extern void memory_stats();
#else
#define w_malloc malloc
#define w_free free
#define memory_stats() ;
#endif
