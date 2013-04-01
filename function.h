#pragma once

#include "value.h"

struct atoken;
struct closure;


struct function
{
	struct value _base;
	bool native;
};


struct function__lambda
{
	struct function _base;
	
	int nargs;
	struct atoken* body;
	struct closure* closure;
};
struct function__native
{
	struct function _base;
	
	char* name;
	struct value* (*func)(int, struct value**);
};



struct function* function_create_lambda (int nargs, struct atoken* body, struct closure* closure);
struct function* function_create_native (char* name, struct value* (*func)(int, struct value**));

void function_register_native (char* name, struct value* (*func)(int, struct value**));
extern void register_native_functions();

struct value* function_apply (struct function* f, int argc, struct value** argv);