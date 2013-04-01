#pragma once

enum token_type
{
	token_number = 1,
	token_symbol = 2,
	token_group = 3,
	
	token_rparen = 4
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
struct token__group
{
	struct token _base;
	struct token** items;
	int length;
};






struct linked_list;

struct token* token_create_number (number_t value);
struct token* token_create_symbol (const char* name);
struct token* token_create_group (struct linked_list* items);


void token_destroy (struct token* t);
void token_display (struct token* t, int indent);
