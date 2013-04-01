#include "include.h"
#include "token.h"
#include "lists.h"



struct token* token_create_number (number_t value)
{
	struct token__number* n = w_malloc(sizeof(struct token__number));
	n->_base.type = token_number;
	n->_base.next = NULL;
	n->value = value;
	return &n->_base;
}
struct token* token_create_symbol (const char* name)
{
	struct token__symbol* n = w_malloc(sizeof(struct token__number) + strlen(name) + 1);
	n->_base.type = token_symbol;
	n->_base.next = NULL;
	n->name = (char*)(n + 1);
	strcpy(n->name, name);
	return &n->_base;
}
struct token* token_create_string (const char* s)
{
	struct token__string* n = w_malloc(sizeof(struct token__string) + strlen(s) + 1);
	n->_base.type = token_string;
	n->_base.next = NULL;
	n->str = (char*)(n + 1);
	strcpy(n->str, s);
	return &n->_base;
}
struct token* token_create_group (struct linked_list* litems)
{
	int i, len = linked_list_length(litems);
	
	struct token__group* g = w_malloc(sizeof(struct token__group) + (len * sizeof(struct token*)));
	g->_base.type = token_group;
	g->_base.next = NULL;
	g->length = len;
	g->items = (struct token**)(g + 1);
	for (i = 0; i < len; i++, litems = litems->next)
		g->items[i] = (struct token*)litems->next->data;
	return &g->_base;
}


void token_destroy (struct token* t)
{
	w_free(t);
}


void token_display (struct token* t, int indent)
{
	if (indent)
		printf("%*c", indent, ' ');
	
	switch (t->type)
	{
		case token_number:
			printf("[-Number-] %f\n", ((struct token__number*)t)->value);
			break;
			
		case token_symbol:
			printf("[-Symbol-] '%s'\n", ((struct token__symbol*)t)->name);
			break;
		
		case token_group:
		{
			printf("[-Group-] \n");
			//     "   blah"
			struct token__group* group = (struct token__group*)t;
			int i;
			for (i = 0; i < group->length; i++)
				token_display(group->items[i], indent + 2);
			break;
		}
		default:
			printf("[-?-]\n");
			break;
	}
}
