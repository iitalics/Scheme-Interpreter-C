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
	lambda->_base.name = NULL;
	
	lambda->nargs = nargs;
	lambda->body = body;
	lambda->closure = closure_expand(closure, 0);//nargs);
	
	lambda->_base._base.f_free = lambda_free;
	
	return &lambda->_base;
}
struct function* function_create_native (char* name, struct value* (*func)(int, struct value**))
{
	struct function__native* f = value_create(value_function, sizeof(struct function__native) + strlen(name) + 1);
	f->_base.native = true;
	f->_base.name = (char*)(f + 1);
	strcpy(f->_base.name, name);
	f->func = func;
	return &f->_base;
}
void function_register_native (char* name, struct value* (*func)(int, struct value**))
{
	globals_set(name, &function_create_native(name, func)->_base);
}






void function_check_arguments (struct function__lambda* f, int argc)
{
	if (argc != f->nargs)
	{
		char q[100];
		sprintf(q, "Expected %d arguments to lambda function, got %d", f->nargs, argc);
		runtime_error(q);
	}
}

static inline struct value* function_apply_native (struct function__native* f, int argc, struct value** argv)
{
	return f->func(argc, argv);
}
static inline struct value* function_apply_lambda (struct function__lambda* f, int argc, struct value** argv)
{
	function_check_arguments(f, argc);
	
	struct value* result = NULL;
	struct closure* closure = closure_expand(f->closure, argc);
	closure_set(closure, argv, argc);
	
	struct closure_fold* fold = &closure->fold;
	
	fold->did_fold = false;
	fold->vals = NULL;
	fold->func = f;
	fold->disable = false;
	
	result = atoken_evaluate(f->body, closure);
	
	while (fold->did_fold)
	{
		closure_set_release(closure, fold->vals, argc);
		fold->did_fold = false;
		
		result = atoken_evaluate(f->body, closure);
	}
	
	//closure_clear(closure, argc);
	closure_destroy(closure);
	
	if (fold->vals != NULL)
		w_free(fold->vals);
	
	return result;
}

struct value* function_apply (struct function* f, int argc, struct value** argv)
{
	if (f->native)
		return function_apply_native((struct function__native*)f, argc, argv);
	else
		return function_apply_lambda((struct function__lambda*)f, argc, argv);
}