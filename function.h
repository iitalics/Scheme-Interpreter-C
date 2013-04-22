#pragma once

#include "value.h"

struct atoken;
struct closure;


struct function
{
	struct value _base;
	bool native;
	char* name;
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
	
	struct value* (*func)(int, struct value**);
};



struct function* function_create_lambda (int nargs, struct atoken* body, struct closure* closure);
struct function* function_create_native (char* name, struct value* (*func)(int, struct value**));

void function_register_native (char* name, struct value* (*func)(int, struct value**));

extern void register_native_functions ();
extern void register_sdl_functions ();
extern void args_check_all (int argc, struct value** argv, enum value_type type, const char* name);
extern void args_check (int argc, struct value** argv, int nargc, const enum value_type* types, const char* name);

struct value* function_apply (struct function* f, int argc, struct value** argv);
//struct value* function_apply (struct function* f, int argc, struct value** argv);
//#define function_apply(f,a,v) function_apply_closure(f,a,v,NULL)