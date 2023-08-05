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

.PHONY: all clean

all: ncrypt

clean:
	rm -f $(objects) ncrypt

decrypt.o: decrypt.c util.h
encrypt.o: encrypt.c util.h
keypair.o: keypair.c
main.o: main.c command.h decrypt.h encrypt.h keypair.h

ncrypt: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)
