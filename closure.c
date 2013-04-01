#include "include.h"
#include "closure.h"
#include "value.h"

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
void closure_destroy (struct closure* c)
{
	if (c == NULL)
		return;
	
	int i;
	for (i = 0; i < c->size; i++)
		value_release(c->vals[i]);
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