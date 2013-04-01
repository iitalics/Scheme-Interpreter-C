#ifndef INCLUDED_STRING_BUFFER_H
#define INCLUDED_STRING_BUFFER_H



#define STRING_BUFFER_SIZE 64


struct str_buffer
{
	char* str;
	int buffer_size, len;
};

void str_buffer_init (struct str_buffer* b);
void str_buffer_add (struct str_buffer* b, char c);
void str_buffer_append (struct str_buffer* b, const char* s);
void str_buffer_clear (struct str_buffer* b);



struct linked_list {
	struct linked_list* next;
	struct linked_list* last;
	void* data;
};

struct linked_list* linked_list_create ();
void linked_list_destroy (struct linked_list* list);
void linked_list_add (struct linked_list* list, void* data);
int linked_list_length (struct linked_list* list);

void** linked_list_array (struct linked_list* list);

#define linked_list_each(n, l) for (n = (l)->first; n != NULL; n = n->next)



#endif//INCLUDED_STRING_BUFFER_H