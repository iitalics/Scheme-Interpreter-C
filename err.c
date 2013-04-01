#include "include.h"
#include "tokenizer.h"



void parse_error (const char* a)
{
	fprintf(stderr, "Parse error: \n"
	                "   %s\n",
		a);
	exit(-1);
}
void runtime_error (const char* a)
{
	fprintf(stderr, "Runtime error: \n"
	                "   %s\n",
		a);
	exit(-1);
}
void tokens_error (const char* s)
{
	fprintf(stderr, "Parse error [line %d]:\n"
	                "   %s\n",
		t_line_number(), s);
	exit(-1);
}