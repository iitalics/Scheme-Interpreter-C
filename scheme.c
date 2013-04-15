#include "include.h"
#include "scheme.h"
#include "tokenizer.h"
#include "value.h"
#include "function.h"
#include "atoken.h"
#include "globals.h"



void scheme_init ()
{
	//register_sdl_functions();
	// under construction
	register_native_functions();
}
void scheme_repl ()
{
	struct atoken* a;
	struct value* v;
	
	while ((a = tokenizer_next_atoken()) != NULL)
	{
		v = atoken_evaluate(a, NULL);
		
		if (value_get_type(v) != value_void)
		{
			value_display(v);
			printf("\n");
		}
		
		value_release(v);
	}
}
void scheme_cleanup ()
{
	tokenizer_close();
	tokenizer_destroy();
	
	globals_destroy();
	atoken_destroy_all();
}