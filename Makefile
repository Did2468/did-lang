CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -g

SRCDIR  = src
SRCS    = $(SRCDIR)/main.c $(SRCDIR)/lexer.c
TARGET  = mycc

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# Quick smoke test: lex the sample file and dump the token stream
test: $(TARGET)
	./$(TARGET) tests/sample.mylang

clean:
	rm -f $(TARGET)
