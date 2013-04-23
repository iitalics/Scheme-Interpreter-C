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



static bool is_list (struct value* v)
{
	while (v != NULL)
	{
		if (value_get_type(v) != value_pair)
			return false;
		
		value_release(v = value_get_tail(v));
	}
	return true;
}


#ifdef UNSAFE_FAST
void args_check_all(int c, struct value** v, enum value_type t, const char* n) {}
void args_check (int argc, struct value** argv, int nargc, const enum value_type* types, const char* name) {}
#else

static bool is_type (struct value* v, enum value_type type)
{
	switch (type)
	{
		case value_list:
			return is_list(v);
		
		case value_any:
			return true;
		
		case value_integer:
		{
			if (value_get_type(v) != value_number)
				return false;
			number_t a = value_get_number(v);
			return (a - (int)a) == 0;
		}
		
		default:
			return value_get_type(v) == type;
	}
}

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
		case value_string:	return "string";
		case value_list:	return "list";
		case value_integer:	return "integer";
		case value_symbol:	return "symbol";
		case value_quote:	return "quote";
		default: 			return "unknown";
	}
}
void args_check_all (int argc, struct value** argv, enum value_type type, const char* name)
{
	char q[128];
	int i;
	for (i = 0; i < argc; i++)
		if (!is_type(argv[i], type))//value_get_type(argv[i]) != type)
		{
			snprintf(q, 128, "Expected argument to '%s' to be of type %s, got %s",
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
		if (!is_type(argv[i], types[i]))
		{
			snprintf(q, 128, "Expected argument #%d to '%s' to be of type %s, got %s",
				i + 1, name, type_string(types[i]), type_string(value_get_type(argv[i])));
			runtime_error(q);
		}
}
#endif

static struct value** list_items (struct value* v, int* l)
{
	struct value* head;
	struct value* tail;
	struct linked_list* ll = linked_list_create();
	
	while (v != NULL)
	{
		if (value_get_type(v) != value_pair)
			runtime_error("Invalid list, tails must be pairs or nil");
		
		head = value_get_head(v);
		tail = value_get_tail(v);
		
		value_release(head);
		value_release(tail);
		
		linked_list_add(ll, head);
		v = tail;
	}
	
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
	
	if (argc == 1)
		return value_create_number(-value_get_number(argv[0]));
	
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
	number_t n = 1;
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
static struct value* scm_modulo (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_number, value_number }, "sqrt");
	number_t a, b;
	
	a = value_get_number(argv[0]);
	b = value_get_number(argv[1]);
	
	return value_create_number(a - ((int)(a / b)) * b);
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
static struct value* scm_is_symbol (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "symbol?");
	return value_create_bool(value_get_type(argv[0]) == value_symbol);
}
static struct value* scm_is_integer (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "integer?");
	
	if (value_get_type(argv[0]) != value_number)
		return value_create_bool(true);
		
	number_t n = value_get_number(argv[0]);
	
	return value_create_bool(n == (int)n);
}


static struct value* scm_is_list (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_any }, "list?");
	return value_create_bool(is_list(argv[0]));
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
	args_check(argc, argv, 1, (enum value_type[]){ value_pair }, "car");
	return value_get_head(argv[0]);
}
static struct value* scm_cdr (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_pair }, "cdr");
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
	//args_check(argc, argv, 2, (enum value_type[]){ value_string, value_string }, "string-append");
	args_check_all(argc, argv, value_string, "string-append");
	
	if (argc == 0)
		return value_create_string("");
	
	char* strings[argc];
	int i, len = 0, x;
	
	for (i = 0; i < argc; i++)
	{
		strings[i] = value_get_string(argv[i]);
		len += strlen(strings[i]);
	}
	
	char* out = w_malloc(len + 1);
	
	for (i = x = 0; i < argc; i++)
	{
		strcpy(out + x, strings[i]);
		x += strlen(strings[i]);
	}
	out[x] = '\0';
	struct value* s = value_create_string(out);
	w_free(out);
	return s;
}
static struct value* scm_string_length (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_string }, "string-length");
	return value_create_number(strlen(value_get_string(argv[0])));
}
static struct value* scm_substring (int argc, struct value** argv)
{
	enum value_type targs[] = { value_string, value_integer, value_integer };
	if (argc == 3)
		args_check(argc, argv, 3, targs, "substring");
	else
		args_check(argc, argv, 2, targs, "substring");
		
	char* s;
	int start, len = -1;
	struct value* result;
	
	s = value_get_string(argv[0]);
	start = (int)value_get_number(argv[1]);
	if (argc == 3)
		len = (int)value_get_number(argv[2]);
	
	if (len < 0 || (start + len) > strlen(s))
		len = strlen(s) - start;
	
	char* out = w_malloc(len + 1);
	strncpy(out, s + start, len);
	out[len] = '\0';
	
	result = value_create_string(out);
	
	w_free(out);
	return result;
}
static struct value* scm_string_to_symbol (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_string }, "string->symbol");
	return value_create_symbol(value_get_string(argv[0]));
}
static struct value* scm_symbol_to_string (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_symbol }, "symbol->string");
	return value_create_string(value_get_symbol_name(argv[0]));
}


static struct value* scm_apply (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_function, value_list }, "apply");
	int length;
	struct value** args;
	args = list_items(argv[1], &length);
	
	struct value* result = function_apply((struct function*)argv[0], length, args);
	w_free(args);
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
		case value_symbol: // hax
			return strcmp(value_get_string(a), value_get_string(b)) == 0;
			
		default:
			return false;
	}
}
static struct value* scm_is_equal (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_any, value_any }, "equal?");
	
	if (value_get_type(argv[0]) != value_get_type(argv[1]))
		return value_create_bool(false);
	return value_create_bool(eql(argv[0], argv[1]));
}
static struct value* scm_is_eq (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_any, value_any }, "eq?");
	
	if (value_get_type(argv[0]) != value_get_type(argv[1]))
		return value_create_bool(false);
	return value_create_bool(eql(argv[0], argv[1]));
}


static struct value* scm_log (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "log");
	return value_create_number(log(value_get_number(argv[0])));
}
static struct value* scm_cos (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "cos");
	return value_create_number(cos(value_get_number(argv[0])));
}
static struct value* scm_sin (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "sin");
	return value_create_number(sin(value_get_number(argv[0])));
}
static struct value* scm_atan (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_number }, "atan");
	return value_create_number(atan(value_get_number(argv[0])));
}


static struct value* scm_map (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_function, value_list }, "map");
	
	int length, i;
	struct value** items =  list_items(argv[1], &length);
	struct value* out;
	struct value* pair = NULL;
	
	struct value* f_args[1];
	
	for (i = length; i-- > 0; )
	{
		f_args[0] = items[i];
		out = function_apply((struct function*)argv[0], 1, f_args);
		pair = value_create_pair(out, pair);
	}
	
	w_free(items);
	
	return pair;
}

static struct value* scm_append (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_list, value_list }, "append");
	
	int length, i;
	struct value** items =  list_items(argv[0], &length);
	struct value* pair = value_retain(argv[1]);
	
	for (i = length; i-- > 0; )
		pair = value_create_pair(value_retain(items[i]), pair);
	
	w_free(items);
	
	return pair;
}

static struct value* scm_reverse (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_list }, "reverse");
	
	struct value* v;
	struct value* pair = NULL;
	
	for (v = argv[0]; v != NULL; value_release(v = value_get_tail(v)))
		pair = value_create_pair(value_get_head(v), pair);
	
	return pair;
}
static struct value* scm_length (int argc, struct value** argv)
{
	args_check(argc, argv, 1, (enum value_type[]){ value_list }, "length");
	
	struct value* v;
	int length = 0;
	
	for (v = argv[0]; v != NULL; value_release(v = value_get_tail(v)))
		length++;
	
	return value_create_number(length);
}
static struct value* scm_list_ref (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_list, value_integer }, "list-ref");
	
	struct value* v;
	int i = (int)value_get_number(argv[1]);
	
	for (v = argv[0]; v != NULL; value_release(v = value_get_tail(v)))
		if (i == 0)
			return value_get_head(v);
		else
			i--;
	
	return value_create_void();
}
static struct value* scm_for_each (int argc, struct value** argv)
{
	args_check(argc, argv, 2, (enum value_type[]){ value_function, value_list }, "for-each");
	struct value* v;
	
	struct value* f_args;
	
	for (v = argv[1]; v != NULL; value_release(v = value_get_tail(v)))
	{
		f_args = value_get_head(v);
		value_release(function_apply((struct function*)argv[0], 1, &f_args));
		
		value_release(f_args);
	}
	
	return value_create_void();
}



// ======================================= car/cdr variations ===============

static struct value* scm_cdar (int ac, struct value** av)
{
	args_check(ac, av, 1, (enum value_type[]){ value_pair }, "cdar");
	struct value* t = value_get_tail(av[0]);
	value_release(t);
	return scm_car(1, &t);
}
static struct value* scm_cadr (int ac, struct value** av)
{
	args_check(ac, av, 1, (enum value_type[]){ value_pair }, "cadr");
	struct value* t = value_get_head(av[0]);
	value_release(t);
	return scm_cdr(1, &t);
}
static struct value* scm_cddr (int ac, struct value** av)
{
	args_check(ac, av, 1, (enum value_type[]){ value_pair }, "cddr");
	struct value* t = value_get_tail(av[0]);
	value_release(t);
	return scm_cdr(1, &t);
}
static struct value* scm_caar (int ac, struct value** av)
{
	args_check(ac, av, 1, (enum value_type[]){ value_pair }, "caar");
	struct value* t = value_get_head(av[0]);
	value_release(t);
	return scm_car(1, &t);
}



// ----------------------------------------------------------------------------------- //
void register_native_functions ()
{
	function_register_native("map", scm_map);
	function_register_native("reverse", scm_reverse);
	function_register_native("length", scm_length);
	function_register_native("list-ref", scm_list_ref);
	function_register_native("for-each", scm_for_each);
	function_register_native("append", scm_append);
	
	function_register_native("string->number", scm_string_to_number);
	function_register_native("number->string", scm_number_to_string);
	function_register_native("string->symbol", scm_string_to_symbol);
	function_register_native("symbol->string", scm_symbol_to_string);
	function_register_native("string-append", scm_string_append);
	function_register_native("string-length", scm_string_length);
	function_register_native("substring", scm_substring);
	
	function_register_native("and", scm_and);
	function_register_native("or", scm_or);
	function_register_native("not", scm_not);
	
	function_register_native("display", scm_display);
	function_register_native("newline", scm_newline);
	function_register_native("read", scm_read);
	function_register_native("exit", scm_exit);
	
	function_register_native("null?", scm_is_null);
	function_register_native("boolean?", scm_is_bool);
	function_register_native("number?", scm_is_number);
	function_register_native("integer?", scm_is_integer);
	function_register_native("void?", scm_is_void);
	function_register_native("pair?", scm_is_pair);
	function_register_native("symbol?", scm_is_symbol);
	function_register_native("list?", scm_is_list);
	function_register_native("string?", scm_is_string);
	function_register_native("eq?", scm_is_eq);
	function_register_native("equal?", scm_is_equal);
	
	function_register_native("cons", scm_cons);
	function_register_native("car", scm_car);
	function_register_native("cdr", scm_cdr);
	function_register_native("list", scm_list);
	function_register_native("apply", scm_apply);
	
	function_register_native("caar", scm_caar);
	function_register_native("cdar", scm_cdar);
	function_register_native("cadr", scm_cadr);
	function_register_native("cddr", scm_cddr);
	
	function_register_native(">", scm_gr);
	function_register_native("<", scm_ls);
	function_register_native(">=", scm_gre);
	function_register_native("<=", scm_lse);
	function_register_native("!=", scm_neq);
	function_register_native("=", scm_eq);
	
	function_register_native("expt", scm_expt);
	function_register_native("floor", scm_floor);
	function_register_native("sqrt", scm_sqrt);
	function_register_native("remainder", scm_modulo);
	function_register_native("modulo", scm_modulo);
	function_register_native("log", scm_log);
	function_register_native("sin", scm_sin);
	function_register_native("cos", scm_cos);
	function_register_native("atan", scm_atan);
	function_register_native("+", scm_add);
	function_register_native("-", scm_sub);
	function_register_native("*", scm_mult);
	function_register_native("/", scm_div);
	
	
	globals_set("PI", value_create_number(M_PI));
	globals_set("E", value_create_number(M_E));
}
