#pragma once

enum token_type
{
	token_number = 1,
	token_symbol = 2,
	token_group = 3,
	token_string = 4,
	token_quote = 5,
	
	token_rparen = 6
};

struct token
{
	enum token_type type;
	struct token* next;
};

struct token__number
{
	struct token _base;
	number_t value;
};
struct token__symbol
{
	struct token _base;
	char* name;
};
struct token__string
{
	struct token _base;
	char* str;
};
struct token__group
{
	struct token _base;
	struct token** items;
	int length;
};
struct token__quote
{
	struct token _base;
	struct token* token;
};





struct linked_list;

struct token* token_create_number (number_t value);
struct token* token_create_symbol (const char* name);
struct token* token_create_group (struct linked_list* items);
struct token* token_create_string (const char* s);
struct token* token_create_quote (struct token* t);


//void token_destroy (struct token* t);
#define token_destroy w_free
void token_display (struct token* t, int indent);
