CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic -gdwarf-4
LFLAGS = -lm

all: encode decode

encode: encode.o io.o trie.o word.o
	$(CC) -o $@ $^ $(LFLAGS)

decode: decode.o io.o trie.o word.o
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f encode decode *.o
	
scan-build: clean
	scan-build --use-cc=$(CC) make

format:
	clang-format -i -style=file *.[ch]
