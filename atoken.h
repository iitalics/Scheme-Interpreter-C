#pragma once


struct value; struct closure; struct token; struct closure_proto;



typedef struct value* (*atoken_eval_function)(void*, struct closure*);
typedef void (*atoken_destroy_function)(void*);

struct atoken
{
	void* data;
	
	atoken_eval_function f_eval;
	atoken_destroy_function f_destroy;
	
	struct atoken* next;
};





// use
struct value* atoken_evaluate (struct atoken* at, struct closure* c);
void atoken_destroy (struct atoken* at);


struct atoken* atoken_const (struct value* value);


extern bool atoken_parse_group (const char* name, int argc, struct token** argv, struct closure_proto* proto, struct atoken** out);
struct atoken* atoken_parse (struct token* token, struct closure_proto* proto);




// mem management
struct atoken* atoken_push (struct atoken* t);
void atoken_destroy_all ();