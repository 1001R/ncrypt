LDFLAGS = -lsodium
ifdef DEBUG
  CFLAGS = -g -Og
else
  CFLAGS = -O2 -Wall 
endif
ifdef STATIC
  LDFLAGS += -static
endif
CFLAGS +=-Wno-unused-result

objects = main.o encrypt.o decrypt.o util.o keypair.o
tests = tests/t_*.c

.PHONY: all clean test

all: ncrypt

clean:
	rm -f $(objects) ncrypt

decrypt.o: decrypt.c util.h
encrypt.o: encrypt.c util.h
keypair.o: keypair.c
main.o: main.c command.h decrypt.h encrypt.h keypair.h

ncrypt: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

# tests/%.o: CPPFLAGS += -I..

tests/runtests.c: $(tests) tests/mktest.sh
	tests/mktest.sh >$@

tests/runtests: tests/runtests.o $(tests:.c=.o)
	$(CC) -o $@ $^

tests/t_base58.o: tests/t_base58.c base58.c

test: tests/runtests
	tests/runtests