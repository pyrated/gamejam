CC = gcc
CFLAGS = -DGLEW_STATIC -g -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -std=gnu99
LDFLAGS = -lglfw -lGLEW -lGL -lalut -lm -lvorbisfile

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

BIN = fifa83

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(BIN) 

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) $(BIN)