#include "include.h"
#include "value.h"
#include "function.h"



struct value__userdata
{
	struct value _base;
	
	void* data;
	bool autodelete;
	void (*destroy_func)(void*);
};


static void userdata_destroy (struct value__userdata* u)
{
	if (u->destroy_func != NULL)
		u->destroy_func(u->data);
	
	if (u->autodelete)
		w_free(u->data);
}

struct value* value_create_userdata (void* data, void (*destroy_func)(void*), bool autodelete)
{
	struct value__userdata* value = value_create(value_userdata, sizeof(struct value__userdata));
	value->data = data;
	value->autodelete = autodelete;
	value->destroy_func = destroy_func;
	value->_base.f_free = (void (*)(struct value*))userdata_destroy;
	return &value->_base;
}