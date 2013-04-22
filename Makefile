CC=gcc
LINK=gcc

CFLAGS=-Wall -O3
# -Isdl/include
LFLAGS=-O4 -lm -s
# -Lsdl/lib -lSDL


OUTPUT=scheme

OBJECTS=main.o lists.o token.o tokenizer.o mem.o	\
		value.o closure.o atoken.o group-syntax.o	\
		err.o globals.o function.o native-funcs.o	\
		scheme.o userdata.o
		#sdl-funcs.o




all: $(OUTPUT)
clean:
	rm -rf $(OUTPUT) $(OBJECTS)
rebuild: clean all


$(OUTPUT): $(OBJECTS)
	$(LINK) $(OBJECTS) -o $(OUTPUT) $(LFLAGS)