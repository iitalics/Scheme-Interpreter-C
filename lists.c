#include "include.h"
#include "lists.h"


// !!!!!!!! STR_BUFFER DOES NOT USE W_MALLOC AND W_FREE !!!!!!!!!!!!!!!



/* usage:

{
	struct str_buffer buffer[1];
	str_buffer_init(buffer);
	
	str_buffer_append(buffer, "Hello, world");
	str_buffer_add(buffer, '!');
	
	puts(buffer->str);
	
	free(buffer->str);
}

*/

void str_buffer_init (struct str_buffer* b)
{
	b->buffer_size = STRING_BUFFER_SIZE;
	b->len = 0;
	
	b->str = malloc(STRING_BUFFER_SIZE + 1);
	b->str[0] = '\0';
}
void str_buffer_add (struct str_buffer* b, char c)
{
	if (b->len >= b->buffer_size)
	{
		b->buffer_size += STRING_BUFFER_SIZE;
		char* buffer = malloc(b->buffer_size + 1);
		strcpy(buffer, b->str);
		free(b->str);
		b->str = buffer;
	}
	
	b->str[b->len] = c;
	b->str[++b->len] = '\0';
}
void str_buffer_append (struct str_buffer* b, const char* s)
{
	int i = 0;
	while (s[i])
		str_buffer_add(b, s[i++]);
}
void str_buffer_clear (struct str_buffer* b)
{
	b->len = 0;
	b->str[0] = '\0';
}



// linked_list does
struct linked_list* linked_list_create ()
{
	struct linked_list* list = w_malloc(sizeof(struct linked_list));
	list->next = list->last = NULL;
	return list;
}
void linked_list_destroy (struct linked_list* list)
{
	if (list->next)
		linked_list_destroy(list->next);
	w_free(list);
}
void linked_list_add (struct linked_list* list, void* data)
{
	struct linked_list* l = w_malloc(sizeof(struct linked_list));
	l->data = data;
	l->next = NULL;
	
	if (list->last == NULL)
		list->next = list->last = l;
	else
		list->last->next = l;
	
	list->last = l;
}
int linked_list_length (struct linked_list* list)
{
	int len = 0;
	for (list = list->next; list != NULL; list = list->next)
		len++;
	
	return len;
}
void** linked_list_array (struct linked_list* list)
{
	struct linked_list* node;
	int i, len;
	len = linked_list_length(list);
	void** array = w_malloc(len * sizeof(void*));
	
	node = list->next;
	for (i = 0; i < len; i++, node = node->next)
		array[i] = node->data;
	
	return array;
}
