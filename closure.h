#pragma once


struct value;
struct atoken;


struct closure
{
	int size;
	struct value** vals;
};



struct closure* closure_expand (struct closure* c, int add);
void closure_set (struct closure* c, struct value** vals, int len);
void closure_destroy (struct closure* c);
int closure_size (struct closure* c);




struct closure_proto
{
	struct closure_proto* next;
	char* name;
};


struct closure_proto* closure_proto_push (struct closure_proto* c, const char* name);
struct closure_proto* closure_proto_shift (struct closure_proto* c, int num);
int closure_proto_get (struct closure_proto* c, const char* name);





