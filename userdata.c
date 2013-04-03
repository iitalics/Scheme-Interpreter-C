#include "include.h"
#include "value.h"
#include "function.h"



struct value__userdata
{
	struct value _base;
	
	void* data;
	void (*destroy_func)(void*);
};


static void userdata_destroy (struct value__userdata* u)
{
	if (u->destroy_func != NULL)
		u->destroy_func(u->data);
}

struct value* value_create_userdata (void* data, void (*destroy_func)(void*))
{
	struct value__userdata* value = value_create(value_userdata, sizeof(struct value__userdata));
	value->data = data;
	value->destroy_func = destroy_func;
	value->_base.f_free = (void (*)(struct value*))userdata_destroy;
	return &value->_base;
}
struct value* value_create_userdata_alloc (size_t size, void (*destroy_func)(void*), void** out)
{
	struct value__userdata* value = value_create(value_userdata, sizeof(struct value__userdata) + size);
	value->data = value + 1;
	value->destroy_func = destroy_func;
	value->_base.f_free = (void (*)(struct value*))userdata_destroy;
	
	if (out != NULL)
		*out = value->data;
	
	return &value->_base;
}

void* value_get_userdata (struct value* value)
{
	return ((struct value__userdata*)value)->data;
}