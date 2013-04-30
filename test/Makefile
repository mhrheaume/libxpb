CC = gcc

INCS = -I/usr/include -I/usr/local/include
LIBS = -L/usr/lib -L/usr/local/lib -lxpb -lX11

CFLAGS = -std=c99 -Wall -Werror ${INCS}
LDFLAGS = ${LIBS}

SRC = $(wildcard *.c)
OBJS = $(SRC:.c=.o)
EXEC = test

.PHONY: all
all: ${EXEC}

%.o:%.c
	${CC} ${CFLAGS} -c -o $@ $<

${EXEC}: ${OBJS}
	${CC} ${LDFLAGS} -o ${EXEC} ${OBJS}

.PHONY: clean
clean:
	@rm -f ${EXEC} ${OBJS}
