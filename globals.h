#pragma once



struct value;


bool globals_get (const char* name, struct value** out);
void globals_set (const char* name, struct value* v);






void globals_destroy ();