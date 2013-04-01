#pragma once


struct atoken;
struct token;
struct linked_list;


void tokenizer_load_string (const char* string);
void tokenizer_load_file (const char* filename);
void tokenizer_load_fs (FILE* f);

struct token* tokenizer_next ();
struct atoken* tokenizer_next_atoken ();

void tokenizer_destroy ();
void tokenizer_close ();


struct token* tokens_create_number (number_t value);
struct token* tokens_create_symbol (const char* name);
struct token* tokens_create_group (struct linked_list* items);


bool t_eof ();
char t_peek ();
char t_read ();

int t_line_number ();

