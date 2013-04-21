#pragma once



struct value;

struct global
{
	char* name;
	struct value* value;
	struct global* next;
};



bool globals_get (const char* name, struct value** out);
void globals_set (const char* name, struct value* v);

struct global* global_global (const char* name);




void globals_destroy ();