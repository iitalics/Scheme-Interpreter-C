#include "include.h"
#include "closure.h"
#include "value.h"
#include "function.h"

struct closure* closure_expand (struct closure* c, int add)
{
	int s, i;
	if (c == NULL)			
		s = 0;
	else
		s = c->size;
	
	struct closure* n = w_malloc(sizeof(struct closure) + (s + add) * sizeof(struct value*));
	n->vals = (struct value**)(n + 1);
	n->size = s + add;
	n->fold.func = NULL;
	
	for (i = 0; i < add; i++)
		n->vals[i] = NULL;
	
	for (i = 0; i < s; i++)
		n->vals[add + i] = value_retain(c->vals[i]);
	
	return n;
}
void closure_set (struct closure* c, struct value** vals, int len)
{
	int i;
	for (i = 0; i < len; i++)
		c->vals[i] = value_retain(vals[i]);
}
void closure_set_release (struct closure* c, struct value** vals, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		value_release(c->vals[i]);
		c->vals[i] = vals[i];
	}
}
void closure_clear (struct closure* c, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		value_release(c->vals[i]);
		c->vals[i] = NULL;
	}
}
void closure_set_one (struct closure* c, int i, struct value* v)
{
	value_release(c->vals[i]);
	c->vals[i] = v;
}
void closure_destroy (struct closure* c)
{
	if (c == NULL)
		return;
	
	closure_clear(c, c->size);
	w_free(c);
}
int closure_size (struct closure* c)
{
	if (c == NULL) return 0;
	return c->size;
}


struct closure_proto* closure_proto_push (struct closure_proto* c, const char* name)
{
	struct closure_proto* p = w_malloc(sizeof(struct closure_proto) + strlen(name) + 1);
	p->next = c;
	p->name = (char*)(p + 1);
	strcpy(p->name, name);
	return p;
}
struct closure_proto* closure_proto_shift (struct closure_proto* c, int num)
{
	struct closure_proto* cur;
	for (; num != 0 && c != NULL; num--)
	{
		cur = c;
		c = c->next;
		
		w_free(cur);
	}
	return c;
}
int closure_proto_get (struct closure_proto* c, const char* name)
{
	int i;
	for (i = 0; c != NULL; c = c->next, i++)
		if (strcmp(c->name, name) == 0)
			return i;
	return -1;
}



void closure_fold_set_arguments (struct closure* c, int argc, struct value** vals)
{
	function_check_arguments(c->fold.func, argc);
	
	if (c->fold.vals == NULL)
		c->fold.vals = w_malloc(argc * sizeof(struct value*));
	
	int i;
	for (i = 0; i < argc; i++)
		c->fold.vals[i] = vals[i];
}