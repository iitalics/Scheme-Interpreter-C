#pragma once




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>


#include "mem.h"


typedef double number_t;






void parse_error (const char* a);
void runtime_error (const char* a);
void tokens_error (const char* a);