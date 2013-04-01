CC=gcc
LINK=gcc

CFLAGS=-Wall -O4
LFLAGS=-O4 -lm


OUTPUT=scheme

OBJECTS=main.o lists.o token.o tokenizer.o mem.o value.o closure.o atoken.o group-syntax.o err.o globals.o function.o native-funcs.o scheme.o





all: $(OUTPUT)
clean:
	rm -rf $(OUTPUT) $(OBJECTS)
rebuild: clean all


$(OUTPUT): $(OBJECTS)
	$(LINK) $(LFLAGS) $(OBJECTS) -o $(OUTPUT)