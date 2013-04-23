#include "include.h"
#include "atoken.h"
#include "token.h"
#include "closure.h"
#include "value.h"
#include "globals.h"
#include "function.h"



#define CACHE_GLOBAL_LINK


static void null_destroy (void* data) {}




/////////////////////////////////////////////////// const //////////////////////////////////////////////////


struct atoken* atoken_const (struct value* value)
{
	struct atoken* t = w_malloc(sizeof(struct atoken));
	t->data = value;
	t->f_eval = (atoken_eval_function)value_retain;//at_const_handler_eval;			--> this one is pretty hacky
	t->f_destroy = (atoken_destroy_function)value_release;//at_const_handler_destroy;
	
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
static struct value* at_global_var_link_handler (struct value** value, struct closure* c)
{
	return value_retain(*value);
}





static struct atoken* atoken_global_var (const char* name)
{
	struct global* g = global_global(name);
	struct atoken* t;
	
	if (g == NULL)
	{
		t = w_malloc(sizeof(struct atoken) + strlen(name) + 1);
		t->data = t + 1;
		strcpy((char*)t->data, name);
		t->f_eval = at_global_var_handler_eval;
	}
	else
	{
		t = w_malloc(sizeof(struct atoken));
		t->data = &g->value;
		t->f_eval = (atoken_eval_function)at_global_var_link_handler;
	}
	
	t->f_destroy = null_destroy;
	return atoken_push(t);
}



/////////////////////////////////////////////////// function //////////////////////////////////////////////////
struct function_call
{
	struct atoken* func;
	int argc;
	struct atoken** argv;
};
static struct value* function_call_eval (struct function_call* f, struct closure* c)
{
	struct value* func = atoken_evaluate(f->func, c);
	
	if (func->type != value_function)
		runtime_error("Cannot apply non-function value");
	
	int i;
	struct value* result;
	struct value* args[f->argc];
	bool folding = (c != NULL && c->fold.func == (struct function__lambda*)func && !c->fold.disable);
	bool disabled = false;
	
	if (!folding && c != NULL)
	{
		c->fold.disable = true;
		disabled = true;
	}
	
	
	for (i = 0; i < f->argc; i++)
	{
		args[i] = atoken_evaluate(f->argv[i], c);
		if (disabled)
			c->fold.disable = true;
	}
	
	
	
	if (folding)
	{
		closure_fold_set_arguments(c, f->argc, args);
		c->fold.did_fold = true;
		
		result = NULL; // value discarded anyways
	}
	else
	{
		result = function_apply((struct function*)func, f->argc, args);
		for (i = 0; i < f->argc; i++)
			value_release(args[i]);
	}
	
	if (disabled)
		c->fold.disable = false;
	
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
	
	t->f_eval = (atoken_eval_function)function_call_eval;
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

static bool const_value (const char* name, struct value** out)
{
	if (strcmp(name, "#t") == 0) { *out = value_create_bool(true); return true; }
	if (strcmp(name, "#f") == 0) { *out = value_create_bool(true); return true; }
	if (strcmp(name, "#void") == 0) { *out = value_create_void(); return true; }
	//if (strcmp(name, "nil") == 0) { *out = NULL; return true; }
	return false;
}

struct value* value_quote_token (struct token* token)
{
	switch (token->type)
	{
		case token_number:
			return value_create_number(((struct token__number*)token)->value);
		
		case token_string:
			return value_create_string(((struct token__string*)token)->str);
		
		case token_symbol:
		{
			struct value* v;
			if (const_value(((struct token__symbol*)token)->name, &v))
				return v;
			else
				return value_create_symbol(((struct token__symbol*)token)->name);
		}
			
		case token_group:
		{
			struct token__group* group = (struct token__group*)token;
			int i;
			struct value* pair = NULL;
			for (i = group->length; i-- > 0;)
				pair = value_create_pair(value_quote_token(group->items[i]), pair);
			
			return pair;
		}
		
		case token_quote:
			return value_create_quote(value_quote_token(((struct token__quote*)token)->token));
		
		default: return NULL;
	}
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
		
		case token_string:
			return atoken_const(value_create_string(((struct token__string*)token)->str));
		
		
		case token_symbol:
		{
			const char* name = ((struct token__symbol*)token)->name;
			
			struct value* c;
			if (const_value(name, &c))
				return atoken_const(c);
				
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
					
				// fall through
			}
			
			// function call
			struct atoken* args[group->length];
			int i;
			for (i = 0; i < group->length; i++)
				args[i] = atoken_parse(group->items[i], proto);
			
			return atoken_function_call(args, group->length);
		}
		
		case token_quote:
			return atoken_const(value_quote_token(((struct token__quote*)token)->token));
		
		
		case token_rparen:
			parse_error("Encountered unexpected ')'");
			break;
		
		default:
			break;
	}
	
	parse_error("could not determine how to parse token");
	return NULL;
}
