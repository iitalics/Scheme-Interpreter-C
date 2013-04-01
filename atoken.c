#include "include.h"
#include "atoken.h"
#include "token.h"
#include "closure.h"
#include "value.h"
#include "globals.h"
#include "function.h"



static void null_destroy (void* data) {}




/////////////////////////////////////////////////// const //////////////////////////////////////////////////

static struct value* at_const_handler_eval (void* data, struct closure* c)
{
	return value_retain((struct value*)data);
}
static void at_const_handler_destroy (void* data)
{
	value_release((struct value*)data);
} 
struct atoken* atoken_const (struct value* value)
{
	struct atoken* t = w_malloc(sizeof(struct atoken));
	t->data = value;
	t->f_eval = at_const_handler_eval;
	t->f_destroy = at_const_handler_destroy;
	return atoken_push(t);
}





/////////////////////////////////////////////////// var //////////////////////////////////////////////////
static struct value* at_var_handler_eval (void* data, struct closure* c)
{
	return value_retain(c->vals[*((int*)data)]);
}
static struct atoken* atoken_local_var (int cindex)
{
	struct atoken* t = w_malloc(sizeof(struct atoken) + sizeof(int));
	int* num = (int*)(t + 1);
	t->data = num;
	*num = cindex;
	t->f_eval = at_var_handler_eval;
	t->f_destroy = null_destroy;
	return atoken_push(t);
}
static struct value* at_global_var_handler_eval (void* data, struct closure* c)
{
#define name ((char*)data)
	struct value* v;
	if (globals_get(name, &v))
		return v;
	
	char q[64 + strlen(name)];
	sprintf(q, "Could not find variable '%s'", name);
	runtime_error(q);
	return value_create_void();
	
#undef name
}
static struct atoken* atoken_global_var (const char* name)
{
	struct atoken* t = w_malloc(sizeof(struct atoken) + strlen(name) + 1);
	t->data = t + 1;
	strcpy((char*)t->data, name);
	t->f_eval = at_global_var_handler_eval;
	t->f_destroy = null_destroy;
	return atoken_push(t);
}



/////////////////////////////////////////////////// var //////////////////////////////////////////////////
struct function_call
{
	struct atoken* func;
	int argc;
	struct atoken** argv;
};
static struct value* function_call_eval (void* _f, struct closure* c)
{
//return value_create_nil();


	struct function_call* f = _f; // why.jpg
	
	struct value* func = atoken_evaluate(f->func, c);
	
	if (func->type != value_function)
		runtime_error("Cannot apply non-function value");
	
	struct value* result;
	int i;
	struct value* args[f->argc];
	
	for (i = 0; i < f->argc; i++)
		args[i] = atoken_evaluate(f->argv[i], c);
	
	result = function_apply((struct function*)func, f->argc, args);
	
	for (i = 0; i < f->argc; i++)
		value_release(args[i]);
	value_release(func);
	
	return result;
}
static struct atoken* atoken_function_call (struct atoken** args, int nargs)
{
	int argc = nargs - 1;
	int i;
	
	struct atoken* t = w_malloc(sizeof(struct atoken) + sizeof(struct function_call) + (argc * sizeof(struct atoken*)));
	struct function_call* f = (struct function_call*)(t + 1);
	t->data = f;
	
	f->func = args[0];
	f->argc = argc;
	f->argv = (struct atoken**)(f + 1);
	for (i = 0; i < argc; i++)
		f->argv[i] = args[i + 1];
	
	t->f_eval = function_call_eval;
	t->f_destroy = null_destroy;
	
	return atoken_push(t);
}













// use
struct value* atoken_evaluate (struct atoken* at, struct closure* c)
{
	return at->f_eval(at->data, c);
}
void atoken_destroy (struct atoken* at)
{
	at->f_destroy(at->data);
	w_free(at);
}


static struct atoken* atoken_all = NULL;

struct atoken* atoken_push (struct atoken* t)
{
	t->next = atoken_all;
	atoken_all = t;
	return t;
}
void atoken_destroy_all ()
{
//return;
	struct atoken* a;
	struct atoken* b;
	for (a = atoken_all; a != NULL; a = b)
	{
		b = a->next;
		atoken_destroy(a);
	}
	atoken_all = NULL;
}




struct atoken* atoken_parse (struct token* token, struct closure_proto* proto)
{
	if (token == NULL) return NULL;
	
	switch (token->type)
	{
		case token_number:
		{
			struct value* val = value_create_number(((struct token__number*)token)->value);
			return atoken_const(val);
		}
		
		case token_symbol:
		{
			const char* name = ((struct token__symbol*)token)->name;
			
			if (strcmp(name, "#t") == 0)
				return atoken_const(value_create_bool(true));
			if (strcmp(name, "#f") == 0)
				return atoken_const(value_create_bool(false));
			if (strcmp(name, "#void") == 0)
				return atoken_const(value_create_void());
			if (strcmp(name, "nil") == 0)
				return atoken_const(value_create_nil());
				
			int i = closure_proto_get(proto, name);
			if (i != -1)
				return atoken_local_var(i);
			
			return atoken_global_var(name);
		}
			
		case token_group:
		{
			struct token__group* group = (struct token__group*)token;
			
			if (group->length == 0)
				return atoken_const(value_create_nil());
			
			if (group->items[0]->type == token_symbol)
			{
				struct atoken* out;
				if (atoken_parse_group(((struct token__symbol*)group->items[0])->name, group->length - 1, group->items + 1, proto, &out))
					return out;
			}
			
			// function call
			struct atoken* args[group->length];
			int i;
			for (i = 0; i < group->length; i++)
				args[i] = atoken_parse(group->items[i], proto);
			
			return atoken_function_call(args, group->length);
		}
		
		default:
			break;
	}
	
	parse_error("could not determine how to parse token");
	return NULL;
}