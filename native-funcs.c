#include "include.h"
#include "function.h"
#include "value.h"
#include "globals.h"
#include "lists.h"


/*
 *
 * Most of this file was generated using scripts I made
 * 
 */


static const char* type_string (enum value_type v)
{
	switch (v)
	{
		case value_number:	return "number";
		case value_nil:		return "nil";
		case value_void:	return "void";
		case value_bool:	return "bool";
		case value_function:return "function";
		case value_pair:	return "pair";
		case value_userdata:return "userdata";
		default: 			return "unknown";
	}
}


void args_check_all (int argc, struct value** argv, enum value_type type, const char* name)
{
	char q[128];
	int i;
	for (i = 0; i < argc; i++)
		if (value_get_type(argv[i]) != type)
		{
			snprintf(q, 128, "Expected arguments to '%s' to be of type %s, got %s",
				name, type_string(type), type_string(value_get_type(argv[i])));
			runtime_error(q);
		}
}
void args_check (int argc, struct value** argv, int nargc, const enum value_type* types, const char* name)
{
	char q[128];
	if (argc != nargc)
	{
		snprintf(q, 128, "Expected %d argument%s to function '%s', got %d", nargc, nargc == 1 ? "" : "s", name, argc);
		runtime_error(q);
	}
	int i;
	for (i = 0; i < argc; i++)
		if (types[i] != value_any && types[i] != value_get_type(argv[i]))
		{
			snprintf(q, 128, "Expected argument #%d to '%s' to be of type %s, got %s",
				i + 1, name, type_string(types[i]), type_string(value_get_type(argv[i])));
			runtime_error(q);
		}
}

static struct value** list_items (struct value* v, int* l)
{
	struct linked_list* ll = linked_list_create();
	while (value_get_type(v) == value_pair)
	{
		linked_list_add(ll, value_get_head(v));
		v = value_get_tail(v);
	}
	if (v != NULL)
		linked_list_add(ll, v);
	struct value** result = (struct value**)linked_list_array(ll);
	*l = linked_list_length(ll);
	linked_list_destroy(ll);
	return result;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static struct value* scm_add (int argc, struct value** argv)
{
	args_check_all(argc, argv, value_number, "+");
	number_t n = 0;
	int i;
	for (i = 0; i < argc; i++)
		n += value_get_number(argv[i]);
	return value_create_number(n);
}
static struct value* scm_mult (int argc, struct value** argv)
{
	args_check_all(argc, argv, value_number, "*");
	number_t n = 1;
	int i;
	for (i = 0; i < argc; i++)
		n *= value_get_number(argv[i]);
	return value_create_number(n);
}
static struct value* scm_sub (int argc, struct value** argv)
{
	args_check_all(argc, argv, value_number, "-");
	number_t n = 0;
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0)
			n = value_get_number(argv[i]);
		else
			n -= value_get_number(argv[i]);
	}
	return value_create_number(n);
}
static struct value* scm_div (int argc, struct value** argv)
{
	args_check_all(argc, argv, value_number, "/");
	number_t n = 0;
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0)
			n = value_get_number(argv[i]);
		else
			n /= value_get_number(argv[i]);
	}
	return value_create_number(n);
}

static struct value* scm_expt (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, "expt");
	return value_create_number(pow(value_get_number(argv[0]), value_get_number(argv[1])));
}
static struct value* scm_sqrt (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "sqrt");
	return value_create_number(sqrt(value_get_number(argv[0])));
}




static struct value* scm_gr (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, ">");
	return value_create_bool(value_get_number(argv[0]) > value_get_number(argv[1]));
}
static struct value* scm_ls (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, "<");
	return value_create_bool(value_get_number(argv[0]) < value_get_number(argv[1]));
}
static struct value* scm_gre (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, ">=");
	return value_create_bool(value_get_number(argv[0]) >= value_get_number(argv[1]));
}
static struct value* scm_lse (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, "<=");
	return value_create_bool(value_get_number(argv[0]) <= value_get_number(argv[1]));
}
static struct value* scm_neq (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, "!=");
	return value_create_bool(value_get_number(argv[0]) != value_get_number(argv[1]));
}
static struct value* scm_eq (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, "=");
	return value_create_bool(value_get_number(argv[0]) == value_get_number(argv[1]));
}

static struct value* scm_and (int argc, struct value** argv)
{
	args_check_all(argc, argv, value_bool, "and");
	bool result = false;
	int i;
	for (i = 0; i < argc; i++)
		result = result && value_get_bool(argv[i]);
	return value_create_bool(result);
}
static struct value* scm_or (int argc, struct value** argv)
{
	args_check_all(argc, argv, value_bool, "and");
	bool result = false;
	int i;
	for (i = 0; i < argc; i++)
		result = result || value_get_bool(argv[i]);
	return value_create_bool(result);
}
static struct value* scm_not (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_bool }, "not");
	return value_create_bool(!value_get_bool(argv[0]));
}




static struct value* scm_is_null (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "null?");
	return value_create_bool(argv[0] == NULL);
}
static struct value* scm_is_bool (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "boolean?");
	return value_create_bool(value_get_type(argv[0]) == value_bool);
}
static struct value* scm_is_number (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "number?");
	return value_create_bool(value_get_type(argv[0]) == value_number);
}
static struct value* scm_is_void (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "void?");
	return value_create_bool(value_get_type(argv[0]) == value_void);
}
static struct value* scm_is_pair (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "pair?");
	return value_create_bool(value_get_type(argv[0]) == value_pair);
}
static struct value* scm_is_string (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "string?");
	return value_create_bool(value_get_type(argv[0]) == value_string);
}
static struct value* scm_is_integer (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "integer?");
	
	if (value_get_type(argv[0]) != value_number)
		return value_create_bool(true);
		
	number_t n = value_get_number(argv[0]);
	
	return value_create_bool(n == (int)n);
}


static struct value* scm_cons (int argc, struct value** argv)
{
	if (argc != 1 && argc != 2)
	{
		char q[128];
		snprintf(q, 128, "Expected 1 or 2 arguments to function 'cons', got %d", argc);
		runtime_error(q);
	}
	
	if (argc == 1)
		return value_create_pair(value_retain(argv[0]), NULL);
		
	return value_create_pair(value_retain(argv[0]), value_retain(argv[1]));
}
static struct value* scm_car (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_pair }, "cat");
	return value_get_head(argv[0]);
}
static struct value* scm_cdr (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_pair }, "cat");
	return value_get_tail(argv[0]);
}
static struct value* scm_list (int argc, struct value** argv)
{
	struct value* pair = NULL;
	int i;
	
	for (i = argc; i-- > 0;)
		pair = value_create_pair(value_retain(argv[i]), pair);
	
	return pair;
}

static struct value* scm_floor (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "floor");
	return value_create_number((int)(value_get_number(argv[0])));
}



static struct value* scm_display (int argc, struct value** argv)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		if (value_get_type(argv[i]) == value_string)
			printf("%s", value_get_string(argv[i]));
		else
			value_display(argv[i]);
	}
	
	return value_create_void();
}
static struct value* scm_newline (int argc, struct value** argv)
{
	printf("\n");
	return value_create_void();
}
static struct value* scm_read (int argc, struct value** argv)
{
	struct str_buffer buffer[1];
	str_buffer_init(buffer);
	char c;
	while ((c = getchar()) != '\n' && c)
		str_buffer_add(buffer, c);
	
	struct value* result = value_create_string(buffer->str);
	free(buffer->str);
	return result;
}
static struct value* scm_exit (int argc, struct value** argv)
{
	if (argc > 0)
	{
		switch (value_get_type(argv[0]))
		{
			case value_number:
				exit((int)(value_get_number(argv[0])));
				
			case value_string:
				printf("%s\n", value_get_string(argv[0]));
				exit(0);
				
			default:
				value_display(argv[0]);
				printf("\n");
				exit(0);
		}
	}
	
	exit(0);
}
static struct value* scm_number_to_string (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "number->string");
	char output[128];
	
	number_t n = value_get_number(argv[0]);
	int base = (int)n;
	n -= base;
	
	char buffer[NUMBER_DIGITS + 2];
	buffer[0] = '\0';
	
	if (n > 0)
	{
		int i;
		for (i = 0; i < NUMBER_DIGITS && n > 0; i++)
		{
			n *= 10;
			buffer[i + 1] = '0' + (int)n;
			n -= (int)n;
		}
		buffer[i + 1] = '\0';
		while (i > 1 && buffer[i] == '0')
			buffer[i--] = '\0';
		
		if (i > 0)
		{
			buffer[0] = '.';
		}
	}
	
	sprintf(output, "%d%s", base, buffer);
	
	return value_create_string(output);
}
static struct value* scm_string_to_number (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_string }, "string->number");
	return value_create_number(atof(value_get_string(argv[0])));
}
static struct value* scm_string_append (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_string, value_string }, "string-append");
	
	char* a = value_get_string(argv[0]);
	char* b = value_get_string(argv[1]);
	char out[strlen(a) + strlen(b) + 1];
	
	strcpy(out, a);
	strcpy(out + strlen(a), b);
	
	return value_create_string(out);
}
static struct value* scm_string_length (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_string }, "string-length");
	return value_create_number(strlen(value_get_string(argv[0])));
}


static struct value* scm_apply (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_function, value_pair }, "apply");
	int length;
	struct value** args;
	args = list_items(argv[1], &length);
	
	struct value* result = function_apply((struct function*)argv[0], length, args);
	free(args);
	return result;
}
static bool eql (struct value* a, struct value* b)
{
	switch (value_get_type(a))
	{
		case value_nil:
		case value_bool:
		case value_void:
			return a == b;
		
		case value_number:
			return value_get_number(a) == value_get_number(b);
		
		case value_pair:
			return eql(value_get_head(a), value_get_head(b)) && eql(value_get_tail(a), value_get_tail(b));
			
		case value_string:
			return strcmp(value_get_string(a), value_get_string(b)) == 0;
			
		default:
			return false;
	}
}
static struct value* scm_equal (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_any, value_any }, "eql?");
	
	if (value_get_type(argv[0]) != value_get_type(argv[1]))
		return value_create_bool(false);
	
	return value_create_bool(eql(argv[0], argv[1]));
}




// ----------------------------------------------------------------------------------- //
void register_native_functions ()
{
	function_register_native("and", scm_and);
	function_register_native("or", scm_or);
	function_register_native("not", scm_not);
	
	function_register_native("display", scm_display);
	function_register_native("newline", scm_newline);
	function_register_native("read", scm_read);
	function_register_native("exit", scm_exit);
	
	function_register_native("string->number", scm_string_to_number);
	function_register_native("number->string", scm_number_to_string);
	function_register_native("string-append", scm_string_append);
	function_register_native("string-length", scm_string_length);
	
	function_register_native("null?", scm_is_null);
	function_register_native("boolean?", scm_is_bool);
	function_register_native("number?", scm_is_number);
	function_register_native("integer?", scm_is_integer);
	function_register_native("void?", scm_is_void);
	function_register_native("pair?", scm_is_pair);
	function_register_native("string?", scm_is_string);
	function_register_native("eql?", scm_equal);
	
	function_register_native("cons", scm_cons);
	function_register_native("car", scm_car);
	function_register_native("cdr", scm_cdr);
	function_register_native("list", scm_list);
	function_register_native("apply", scm_apply);
	
	function_register_native(">", scm_gr);
	function_register_native("<", scm_ls);
	function_register_native(">=", scm_gre);
	function_register_native("<=", scm_lse);
	function_register_native("!=", scm_neq);
	function_register_native("=", scm_eq);
	
	function_register_native("expt", scm_expt);
	function_register_native("floor", scm_floor);
	function_register_native("sqrt", scm_sqrt);
	function_register_native("+", scm_add);
	function_register_native("-", scm_sub);
	function_register_native("*", scm_mult);
	function_register_native("/", scm_div);
}
