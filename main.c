#include "include.h"
#include "scheme.h"
#include "tokenizer.h"
#include "lists.h"




static void repl ()
{
	fprintf(stderr, "\n  ~=+ Scheme REPL v1.0 +=~\n\n");
	
	scheme_init();
	
	struct str_buffer buffer[1];
	str_buffer_init(buffer);
	
	char c;
	bool eof = false;
	
	for (;;)
	{
		printf("> ");
		while ((c = getchar()) != '\n' && c)
		{
			if (feof(stdin)
#ifdef WIN32
				|| (c == 4)
#endif
			)
			{
				eof = true;
				break;
			}


			str_buffer_add(buffer, c);
		}
		
		if (c != '\n')
			printf("\n");
		
		tokenizer_load_string(buffer->str);
		scheme_repl();
		
		str_buffer_clear(buffer);
		
		if (eof)
			break;
	}
	
	free(buffer->str);
	
	scheme_cleanup();
	
	memory_stats();
}
static void file (FILE* fs)
{
	scheme_init();
	tokenizer_load_fs(fs);
	scheme_repl();
	scheme_cleanup();
	
	memory_stats();
}

static int help_text (const char* n)
{
	printf(" Usage: %s [FILE]\n"
	       "\n"
		   "   Execute scheme file or choose no file for interactive REPL\n",
		n);
	return 0;
}

int main (int argc, char** argv)
{
	if (argc == 0) return -1;
	
	if (argc == 1)
		repl();
	else
	{
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
			return help_text(argv[0]);
		
		FILE* fs = fopen(argv[1], "r");
		
		if (fs)
			file(fs);
		else
		{
			fprintf(stderr, "Could not open file '%s'\n", argv[1]);
			exit(-1);
		}
	}
	
	return 0;
}