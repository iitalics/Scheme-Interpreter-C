#include "include.h"
#include "function.h"
#include "value.h"
#include "atoken.h"
#include "closure.h"
#include "globals.h"



static void lambda_free (struct value* v)
{
	struct function__lambda* lambda = (struct function__lambda*)v;
	closure_destroy(lambda->closure);
}

struct function* function_create_lambda (int nargs, struct atoken* body, struct closure* closure)
{
	struct function__lambda* lambda = value_create(value_function, sizeof(struct function__lambda));
	lambda->_base.native = false;
	
	lambda->nargs = nargs;
	lambda->body = body;
	lambda->closure = closure_expand(closure, 0);
	
	lambda->_base._base.f_free = lambda_free;
	
	return &lambda->_base;
}
struct function* function_create_native (char* name, struct value* (*func)(int, struct value**))
{
	struct function__native* f = value_create(value_function, sizeof(struct function__native) + strlen(name) + 1);
	f->_base.native = true;
	f->name = (char*)(f + 1);
	strcpy(f->name, name);
	f->func = func;
	return &f->_base;
}
void function_register_native (char* name, struct value* (*func)(int, struct value**))
{
	globals_set(name, &function_create_native(name, func)->_base);
}







static inline struct value* function_apply_native (struct function__native* f, int argc, struct value** argv)
{
	return f->func(argc, argv);
}
static struct value* function_apply_lambda (struct function__lambda* f, int argc, struct value** argv)
{
	if (argc != f->nargs)
	{
		char q[100];
		sprintf(q, "Expected %d arguments to lambda function, got %d", f->nargs, argc);
		runtime_error(q);
	}
	
	struct closure* closure = closure_expand(f->closure, argc);
	closure_set(closure, argv, argc);
	
	struct value* result = atoken_evaluate(f->body, closure);
	
	closure_destroy(closure);
	
	return result;
}

struct value* function_apply (struct function* f, int argc, struct value** argv)
{
	if (f->native)
		return function_apply_native((struct function__native*)f, argc, argv);
	else
		return function_apply_lambda((struct function__lambda*)f, argc, argv);
}