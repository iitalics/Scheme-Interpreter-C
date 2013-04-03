#pragma once


enum value_type
{
	value_nil = 0,
	value_void = 1,
	
	value_number = 2,
	value_bool = 3,
	value_function = 4,
	value_pair = 5,
	value_string = 6,
	value_userdata = 7,
	
	value_any = -1
};


#define NUMBER_DIGITS 15

struct value
{
	enum value_type type;
	int rcount;
	bool constant;
	void (*f_free)(struct value*);
};

void* value_create (enum value_type type, size_t s);
struct value* value_create_number (number_t value);
#define value_create_nil() NULL
struct value* value_create_void ();
struct value* value_create_bool (bool b);
struct value* value_create_pair (struct value* a, struct value* b);
struct value* value_create_string (const char* s);

// suggested you use 'w_free' for 'destroy_func' if you just want to deallocate your memory
struct value* value_create_userdata (void* data, void (*destroy_func)(void*));
struct value* value_create_userdata_alloc (size_t size, void (*destroy_func)(void*), void** out);
void value_destroy (struct value* v);

struct value* value_retain (struct value* v);
bool value_release (struct value* v);


enum value_type value_get_type (struct value* value);
number_t value_get_number (struct value* value);
struct value* value_get_head (struct value* pair);
struct value* value_get_tail (struct value* pair);
bool value_get_bool (struct value* value);
bool value_is_null (struct value* value);
char* value_get_string (struct value* value);
void* value_get_userdata (struct value* value);


void value_display (struct value* value);
