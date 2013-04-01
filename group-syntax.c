#include "include.h"
#include "atoken.h"
#include "token.h"
#include "closure.h"
#include "value.h"
#include "globals.h"
#include "function.h"


typedef struct value* (*eval_func) (void*, struct closure*);
typedef void (*destroy_func) (void*);


static void NULL_DESTROY (void* data) {}



static struct atoken* create_atoken (size_t extra_size, void** out)
{
	struct atoken* a = w_malloc(sizeof(struct atoken) + extra_size);
	a->data = (void*)(a + 1);
	if (out)
		*out = a->data;
	return a;
}



struct if_struct
{
	struct atoken* cond;
	struct atoken* clause_then;
	struct atoken* clause_else;
};
static struct value* if_eval (struct if_struct* data, struct closure* c)
{
	struct value* cond = atoken_evaluate(data->cond, c);
	
	if (value_get_type(cond) != value_bool)
		runtime_error("Expected 'if' statement condition to be boolean");
	
	bool v = value_get_bool(cond);
	
	value_release(cond);
	
	if (v)
		return atoken_evaluate(data->clause_then, c);
	else
		return atoken_evaluate(data->clause_else, c);
}



struct define_struct
{
	char* name;
	struct atoken* body;
};
static struct value* define_eval (struct define_struct* data, struct closure* c)
{
	globals_set(data->name, atoken_evaluate(data->body, c));
	return value_create_void();
}



struct lambda_struct
{
	int argc;
	struct atoken* body;
};
static struct value* lambda_eval (struct lambda_struct* data, struct closure* c)
{
	return &function_create_lambda(data->argc, data->body, c)->_base;
}
static struct atoken* create_lambda (int argc, struct token** argv, struct token* body, struct closure_proto* proto)
{
	struct lambda_struct* s;
	struct atoken* token = create_atoken(sizeof(struct lambda_struct), (void**)&s);
	s->argc = argc;
	
	struct closure_proto* p = proto;
	int i;
	for (i = argc; i-- > 0;)
	{
		if (argv[i]->type != token_symbol)
			parse_error("Expected argument names all to symbols");
		
		p = closure_proto_push(p, ((struct token__symbol*)argv[i])->name);
	}
	
	s->body = atoken_parse(body, p);
	
	closure_proto_shift(p, argc);
	
	token->f_eval = (eval_func)lambda_eval;
	token->f_destroy = NULL_DESTROY;
	
	return atoken_push(token);
}



struct cond_struct
{
	int num_conds;
	struct atoken** conds;
	struct atoken** thens;
};
static struct value* cond_eval (struct cond_struct* s, struct closure* c)
{
	int i;
	struct value* condition;
	bool b;
	
	for (i = 0; i < s->num_conds; i++)
	{
		if (s->conds[i] == NULL)
			b = true;
		else
		{
			condition = atoken_evaluate(s->conds[i], c);
			
			if (value_get_type(condition) != value_bool)
				runtime_error("Conditions in 'cond' must all be boolean");
			
			b = value_get_bool(condition);
			value_release(condition);
		}
		
		if (b)
			return atoken_evaluate(s->thens[i], c);
	}
	
	return value_create_void();
}

struct let_struct
{
	int num_vars;
	struct atoken** values;
	struct atoken* body;
};
static struct value* let_eval (struct let_struct* s, struct closure* c)
{
	struct closure* closure = closure_expand(c, s->num_vars);
	int i;
	for (i = 0; i < s->num_vars; i++)
		closure->vals[i] = atoken_evaluate(s->values[i], closure);
	
	struct value* result = atoken_evaluate(s->body, closure);
	
	closure_destroy(closure);
	
	return result;
}
static struct atoken* create_let (struct token** vars, int nvars, struct token* body, struct closure_proto* proto)
{
	struct atoken* result;
	struct let_struct* s;
	int i;
	
	result = create_atoken(sizeof(struct let_struct) + (nvars * sizeof(struct atoken*)), (void**)&s);
	result->f_eval = (eval_func)let_eval;
	result->f_destroy = NULL_DESTROY;
	s->num_vars = nvars;
	s->values = (struct atoken**)(s + 1);
	
	for (i = nvars; i-- > 0;)
	{
		if (vars[i]->type != token_group)
			parse_error("Variable in 'let' must be declared as group");
		if (((struct token__group*)vars[i])->items[0]->type != token_symbol)
			parse_error("Variable name in 'let' must be symbol");
		
		proto = closure_proto_push(proto, ((struct token__symbol*)((struct token__group*)vars[i])->items[0])->name);
		//printf("declared variable '%s'\n", ((struct token__symbol*)((struct token__group*)vars[i])->items[0])->name);
	}
	
	for (i = 0; i < nvars; i++)
		s->values[i] = atoken_parse(((struct token__group*)vars[i])->items[1], proto);
	
	s->body = atoken_parse(body, proto);
	
	proto = closure_proto_shift(proto, nvars);
	
	return atoken_push(result);
}









bool atoken_parse_group (const char* name, int argc, struct token** argv, struct closure_proto* proto, struct atoken** out)
{
	struct atoken* result;
	
	if (strcmp(name, "if") == 0)
	{
		if (argc != 3)
			parse_error("Expected 3 arguments to 'if' syntax");
			
		struct if_struct* s;
		result = create_atoken(sizeof(struct if_struct), (void**)&s);
		result->f_eval = (eval_func)if_eval;
		result->f_destroy = NULL_DESTROY;
		s->cond = atoken_parse(argv[0], proto);
		s->clause_then = atoken_parse(argv[1], proto);
		s->clause_else = atoken_parse(argv[2], proto);
		*out = atoken_push(result);
		return true;
	}
	if (strcmp(name, "define") == 0)
	{
		if (argc != 2)
			parse_error("Expected 2 arguments to 'define' syntax");
		
		struct define_struct* s;
		char* name;
		struct atoken* body;
		
		if (argv[0]->type == token_group)
		{
			struct token__group* group = (struct token__group*)argv[0];
			
			if (group->length == 0)
				parse_error("Invalid empty list as 'define' name");
			if (group->items[0]->type != token_symbol)
				parse_error("Define function name must be symbol");
			
			name = ((struct token__symbol*)group->items[0])->name;
			body = create_lambda(group->length - 1, group->items + 1, argv[1], proto);
		}
		else if (argv[0]->type == token_symbol)
		{
			name = ((struct token__symbol*)argv[0])->name;
			body = atoken_parse(argv[1], proto);
		}
		else
		{
			parse_error("Expected group or symbol as first argument to 'define'");
			return false;
		}
		
		result = create_atoken(sizeof(struct define_struct) + strlen(name) + 1, (void**)&s);
		result->f_eval = (eval_func)define_eval;
		result->f_destroy = NULL_DESTROY;
		s->name = (char*)(s + 1);
		strcpy(s->name, name);
		s->body = body;
		*out = atoken_push(result);
		return true;
	}
	if (strcmp(name, "lambda") == 0)
	{
		if (argc != 2)
			parse_error("Expected 2 arguments to 'lambda' syntax");
		if (argv[0]->type != token_group)
			parse_error("Expected group as lambda argument list");
		
		struct token__group* group = (struct token__group*)argv[0];
		
		*out = create_lambda(group->length, group->items, argv[1], proto);
		return true;
	}
	if (strcmp(name, "cond") == 0)
	{
		struct cond_struct* s;
		int i;
		result = create_atoken(sizeof(struct cond_struct) + (argc * sizeof(struct atoken*) * 2), (void**)&s);
		
		s->num_conds = argc;
		s->conds = (struct atoken**)(s + 1);
		s->thens = s->conds + argc;
		
		for (i = 0; i < argc; i++)
		{
			if (argv[i]->type != token_group)
				parse_error("Clauses in 'cond' must be groups");
				
				
			struct token__group* group = (struct token__group*)argv[i];
			
			if (group->length != 2)
				parse_error("Clauses in 'cond' expect 2 arguments");
			
			if (group->items[0]->type == token_symbol && strcmp(((struct token__symbol*)group->items[0])->name, "else") == 0)
				s->conds[i] = NULL;
			else
				s->conds[i] = atoken_parse(group->items[0], proto);
			
			s->thens[i] = atoken_parse(group->items[1], proto);
		}
		result->f_eval = (eval_func)cond_eval;
		result->f_destroy = NULL_DESTROY;
		*out = atoken_push(result);
		return true;
	}
	if (strcmp(name, "let") == 0)
	{
		if (argc != 2)
			parse_error("Expected 2 arguments to 'let' syntax");
		if (argv[0]->type != token_group)
			parse_error("Variable list in 'let' must be group");
		
		struct token__group* group = (struct token__group*)argv[0];
		*out = create_let(group->items, group->length, argv[1], proto);
		return true;
	}
	
	return false;
}