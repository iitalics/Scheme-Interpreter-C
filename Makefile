CC=gcc
LINK=gcc

CFLAGS=-Wall -O3
# -Isdl/include
LFLAGS=-O4 -lm -s
# -Lsdl/lib -lSDL


OUTPUT=scheme

OBJECTS=obj/main.o obj/lists.o obj/token.o obj/tokenizer.o obj/mem.o	\
		obj/value.o obj/closure.o obj/atoken.o obj/group-syntax.o	\
		obj/err.o obj/globals.o obj/function.o obj/native-funcs.o	\
		obj/scheme.o obj/userdata.o
		#sdl-funcs.o



obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(OUTPUT)
clean:
	rm -rf $(OUTPUT) $(OBJECTS)
rebuild: clean all


$(OUTPUT): $(OBJECTS)
	$(LINK) $(OBJECTS) -o $(OUTPUT) $(LFLAGS)