#include "include.h"
#include "value.h"
#include "function.h"


struct value__number
{
	struct value _base;
	number_t value;
};

struct value__pair
{
	struct value _base;
	
	struct value* car;
	struct value* cdr;
};



static struct value c_values[] =	// constant values
{
	{	// true
		.type = value_bool, 
		.constant = true
	},
	{	// false
		.type = value_bool, 
		.constant = true
	},
	{	// void
		.type = value_void,
		.constant = true
	}
};

#define c_bool_true (c_values + 0)
#define c_bool_false (c_values + 1)

void* value_create (enum value_type type, size_t s)
{
	struct value* v = w_malloc(s);
	v->constant = false;
	v->rcount = 1;
	v->type = type;
	v->f_free = NULL;
	return v;
}

struct value* value_create_number (number_t value)
{
	struct value__number* v = value_create(value_number, sizeof(struct value__number));
	v->value = value;
	return &v->_base;
}
struct value* value_create_string (const char* s)
{
	struct value* v = value_create(value_string, sizeof(struct value) + strlen(s) + 1);
	strcpy((char*)(v + 1), s);
	return v;
}
struct value* value_create_bool (bool b)
{
	if (b)
		return c_bool_true;
	else
		return c_bool_false;
}
struct value* value_create_void ()
{
	return c_values + 2;
}



static void pair_destroy (struct value* v)
{
	value_release(((struct value__pair*)v)->car);
	value_release(((struct value__pair*)v)->cdr);
}
struct value* value_create_pair (struct value* a, struct value* b)
{
	struct value__pair* v = value_create(value_pair, sizeof(struct value__pair));
	v->car = a;
	v->cdr = b;
	v->_base.f_free = pair_destroy;
	return &v->_base;
}
void value_destroy (struct value* v)
{
	if (v->f_free != NULL)
		v->f_free(v);
	w_free(v);
}

struct value* value_retain (struct value* v)
{
	if (v == NULL || v->constant)
		return v;
	v->rcount++;
	return v;
}
bool value_release (struct value* v)
{
	if (v == NULL || v->constant)
		return false;
	v->rcount--;
	if (v->rcount <= 0)
	{
		value_destroy(v);
		return true;
	}
	return false;
}


enum value_type value_get_type (struct value* value)
{
	if (value == NULL) return value_nil;
	return value->type;
}
number_t value_get_number (struct value* value)
{
	return ((struct value__number*)value)->value;
}
bool value_get_bool (struct value* value)
{
	return value != c_bool_false;
}
struct value* value_get_head (struct value* pair)
{
	return value_retain(((struct value__pair*)pair)->car);
}
struct value* value_get_tail (struct value* pair)
{
	return value_retain(((struct value__pair*)pair)->cdr);
}
char* value_get_string (struct value* value)
{
	return (char*)(value + 1); // haha!
}
bool value_is_null (struct value* value)
{
	return value == NULL;
}



void value_display (struct value* value)
{
	switch (value_get_type(value))
	{
		case value_nil:
			printf("nil");
			break;
			
		case value_number:
		{
			number_t n = value_get_number(value);
			int base = (int)n;
			n -= base;
			
			printf("%d", base);
			
			if (n > 0)
			{
				char buffer[NUMBER_DIGITS + 1];
				int i;
				for (i = 0; i < NUMBER_DIGITS && n > 0; i++)
				{
					n *= 10;
					buffer[i] = '0' + (int)n;
					n -= (int)n;
				}
				buffer[i] = '\0';
				while (i > 0 && buffer[i - 1] == '0')
					buffer[--i] = '\0';
				
				if (i > 0)
					printf(".%s", buffer);
			}
			
			break;
		}
		
		case value_bool:
			if (value == c_bool_true)
				printf("#t");
			else
				printf("#f");
			break;
			
		case value_function:
		{
			struct function* f = (struct function*)value;
			const char* type = f->native ? "Native function" : "Lambda function";
			if (f->name == NULL)
				printf("< %s >", type);
			else
				printf("< %s '%s' >", type, f->name);
			break;
		}
		
		case value_void:
			break;
			
		case value_pair:
		{
			struct value__pair* pair = (struct value__pair*)value;
			printf("(");
			value_display(pair->car);
			
			if (value_get_type(pair->cdr) == value_pair)
			{
				while (value_get_type(pair->cdr) == value_pair)
				{
					pair = (struct value__pair*)(pair->cdr);
					printf(" ");
					value_display(pair->car);
				}
			}
			if (pair->cdr != NULL)
			{
				printf(" . ");
				value_display(pair->cdr);
			}
			printf(")");
			break;
		}
		
		case value_string:
		{
			int i, len;
			char* str = value_get_string(value);
			putchar('"');
			for (i = 0, len = strlen(str); i < len; i++)
			{
				if (str[i] == '"')
					putchar('\\');
				else if (str[i] == '\n')
				{
					printf("\\n");
					continue;
				}
				putchar(str[i]);
			}
			putchar('"');
			break;
		}
		
		case value_userdata:
			printf("< Userdata %p >", value_get_userdata(value));
			break;
		
		default:
			printf("< ? id #%d ? >", value->type);
			break;
	}
}
