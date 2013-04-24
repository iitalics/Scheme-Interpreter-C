#include "include.h"
#include "tokenizer.h"
#include "token.h"
#include "lists.h"
#include "atoken.h"


static void tokens_init ();
static int line_number;
static bool peek_cached = false;
static char peek_cache;


enum read_mode {
	read_mode_none,
	read_mode_file,
	read_mode_string
};
static FILE* read_fhandle;
static char* read_string;
static int read_i;
static enum read_mode read_mode = read_mode_none;


struct tstate
{
	FILE* fh;
	char* str;
	int ri;
	enum read_mode rm;
	int ln;
};

void* tokenizer_pull_state ()
{
	struct tstate* t = w_malloc(sizeof(struct tstate));
	t->fh = read_fhandle;
	t->str = read_string;
	t->ri = read_i;
	t->rm = read_mode;
	t->ln = line_number;
	read_mode = read_mode_none;
	peek_cached = false;
	return t;
}
void tokenizer_return_state (void* v)
{
	struct tstate* t = (struct tstate*)v;
	
	read_fhandle = t->fh;
	read_string = t->str;
	read_i = t->ri;
	read_mode = t->rm;
	line_number = t->ln;
	
	peek_cached = false;
	w_free(v);
}


static struct token* all_tokens = NULL;



static struct token c_rparen[] =
{
	{ .type = token_rparen,
	  .next = NULL }
};


void tokenizer_load_string (const char* string)
{
	if (read_mode != read_mode_none)
		tokenizer_close();
		
	read_string = strdup(string);
	read_mode = read_mode_string;
	read_i = 0;
	
	tokens_init();
}
void tokenizer_load_fs (FILE* f)
{
	if (read_mode != read_mode_none)
		tokenizer_close();
		
	read_fhandle = f;
	read_mode = read_mode_file;
	tokens_init();
}
void tokenizer_load_file (const char* filename)
{
	tokenizer_load_fs(fopen(filename, "r"));
}
void tokenizer_destroy ()
{
	struct token* t;
	struct token* next;
	for (t = all_tokens; t != NULL; t = next)
	{
		next = t->next;
		token_destroy(t);
	}
	all_tokens = NULL;
}
void tokenizer_close ()
{
	if (read_mode == read_mode_file)
		fclose(read_fhandle);
	else if (read_mode == read_mode_string)
		free(read_string);
	
	read_mode = read_mode_none;
}

static void tokens_init ()
{
	line_number = 1;
	peek_cached = false;
}



bool t_eof ()
{
	if (read_mode == read_mode_file)
		return feof(read_fhandle);
		
	return read_string[read_i] == '\0';
}
char t_peek ()
{
	if (t_eof())
		return '\0';
	if (read_mode == read_mode_file)
	{
		if (peek_cached)
			return peek_cache;
		
		char c = getc(read_fhandle);
		ungetc(c, read_fhandle);
		
		peek_cached = true;
		peek_cache = c;
		
		return c;
	}
	else
		return read_string[read_i];
}
char t_read ()
{
	if (t_eof())
		return '\0';
	char out;
	if (read_mode == read_mode_file)
	{
		out = getc(read_fhandle);
		peek_cached = false;
	}
	else
		out = read_string[read_i++];
	if (out == '\n')
		line_number++;
	return out;
}

int t_line_number ()
{
	return line_number;
}

static void trim ()
{
	while (!t_eof() && isspace(t_peek()))
		t_read();
}


static char* read_end ()
{
	struct str_buffer buffer[1];
	str_buffer_init(buffer);
	
	char c;
	
	while (!t_eof() && !isspace(c = t_peek()) && c != ')' && c != '(')
		str_buffer_add(buffer, t_read());
	
	return buffer->str;
}

char read_escape_char ()
{
	switch (t_read())
	{
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case '\"': return '\"';
		case '\'': return '\'';
		case '\\': return '\\';
		
		default:
			tokens_error("Unrecognized escape sequence");
			return '\0';
	}
}

static char* read_quoted_string ()
{
	t_read();
	char c;
	
	struct str_buffer buffer[1];
	str_buffer_init(buffer);
	
	while ((c = t_read()) != '"')
	{
		if (t_eof())
			tokens_error("Reached EOF before closing \"");
		
		if (c == '\\')
			c = read_escape_char();
		
		str_buffer_add(buffer, c);
	}
	
	return buffer->str;
}






struct token* tokenizer_next ()
{
start:
	trim();
	
	if (t_eof())
		return NULL;
	
	if (t_peek() == ';')
	{
		while (!t_eof() && t_read() != '\n') ;
		goto start;
	}
	
	
	switch (t_peek())
	{
		case '(':
		{
			t_read();
			
			struct linked_list* ll = linked_list_create();
			struct token* item;
			
			while ((item = tokenizer_next()) != c_rparen)
			{
				if (item == NULL)
					tokens_error("Reached EOF before closing ')'");
				
				linked_list_add(ll, item);
			}
			
			struct token* group = tokens_create_group(ll);
			
			linked_list_destroy(ll);
			
			return group;
		}
		
		case ')':
			t_read();
			return c_rparen;
			
		case '"':
			return tokens_create_string(read_quoted_string());
		
		case '\'':
		{
			t_read();
			struct token* q = tokenizer_next();
			if (q == NULL)
				tokens_error("Expected token after '");
			
			return tokens_create_quote(q);
		}
		
		default:
		{
			char* block = read_end();
			
			struct token* token;
			
			if (isdigit(block[0]) || block[0] == '.' || ((block[0] == '-' || block[0] == '+') && block[1]))
				token = tokens_create_number(atof(block));
			else
				token = tokens_create_symbol(block);
			
			free(block);
			return token;
		}
	}
}


struct atoken* tokenizer_next_atoken ()
{
	return atoken_parse(tokenizer_next(), NULL);
}











static struct token* push_token (struct token* t)
{
	t->next = all_tokens;
	all_tokens = t;
	return t;
}

struct token* tokens_create_number (number_t value)
{
	struct token* t;
	for (t = all_tokens; t != NULL; t = t->next)
		if (t->type == token_number && ((struct token__number*)t)->value == value)
			return t;
	return push_token(token_create_number(value));
}
struct token* tokens_create_symbol (const char* name)
{
	struct token* t;
	for (t = all_tokens; t != NULL; t = t->next)
		if (t->type == token_symbol && strcmp(((struct token__symbol*)t)->name, name) == 0)
			return t;
	return push_token(token_create_symbol(name));
}
struct token* tokens_create_string (const char* str)
{
	return push_token(token_create_string(str));
}
struct token* tokens_create_group (struct linked_list* items)
{
	return push_token(token_create_group(items));
}
struct token* tokens_create_quote (struct token* t)
{
	return push_token(token_create_quote(t));
}