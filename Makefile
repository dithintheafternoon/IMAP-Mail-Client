CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu99 -lm
LDFLAGS = -lm

SRCS = main.c mail.c
EXE = fetchmail

all: $(EXE)

$(EXE): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean: 
	rm -f $(EXE)