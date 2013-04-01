#pragma once


struct value; struct closure; struct token; struct closure_proto;



struct atoken
{
	void* data;
	
	struct value* (*f_eval)(void*, struct closure*);
	void (*f_destroy)(void*);
	
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