#pragma once


struct value;
struct atoken;
struct function__lambda;

struct closure_fold
{
	bool disable;
	bool did_fold;
	struct value** vals;
	struct function__lambda* func;
};

struct closure
{
	int size;
	struct value** vals;
	struct closure_fold fold;
};




struct closure* closure_expand (struct closure* c, int add);
void closure_set (struct closure* c, struct value** vals, int len);
void closure_set_release (struct closure* c, struct value** vals, int len);
void closure_clear (struct closure* c, int len);
void closure_set_one (struct closure* c, int i, struct value* v);
void closure_destroy (struct closure* c);
int closure_size (struct closure* c);


void closure_fold_set_arguments (struct closure* c, int argc, struct value** vals);




struct closure_proto
{
	struct closure_proto* next;
	char* name;
};


struct closure_proto* closure_proto_push (struct closure_proto* c, const char* name);
struct closure_proto* closure_proto_shift (struct closure_proto* c, int num);
int closure_proto_get (struct closure_proto* c, const char* name);





