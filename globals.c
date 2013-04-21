#include "include.h"
#include "globals.h"
#include "value.h"

static struct global* globals = NULL;





struct global* global_global (const char* name)
{
	struct global* g;
	for (g = globals; g != NULL; g = g->next)
		if (strcmp(g->name, name) == 0)
			return g;
	return NULL;
}
bool globals_get (const char* name, struct value** out)
{
	struct global* g;
	for (g = globals; g != NULL; g = g->next)
		if (strcmp(g->name, name) == 0)
		{
			*out = value_retain(g->value);
			return true;
		}
	return false;
}
void globals_set (const char* name, struct value* v)
{
	struct global* g;
	for (g = globals; g != NULL; g = g->next)
		if (strcmp(g->name, name) == 0)
		{
			value_release(g->value);
			g->value = v;
			return;
		}
	
	g = malloc(sizeof(struct global) + strlen(name) + 1);
	g->name = (char*)(g + 1);
	strcpy(g->name, name);
	g->value = v;
	g->next = globals;
	globals = g;
}




void globals_destroy ()
{
	struct global* g;
	for (; globals != NULL; globals = g)
	{
		g = globals->next;
		value_release(globals->value);
		free(globals);
	}
}