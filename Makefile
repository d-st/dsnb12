#
# Makefile for SPL compiler
#

CC = gcc
CFLAGS = -Wall -Wno-unused -g
LDFLAGS = -g
LDLIBS = -lm

SRCS = main.c utils.c sym.c absyn.c parser.tab.c lex.yy.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = spl

.PHONY:		all tests depend clean dist-clean

all:		$(BIN)

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

parser.tab.c:	parser.y
		bison -t -d parser.y

lex.yy.c:	scanner.l
		flex scanner.l

tests:		all
		@for i in Tests/test??.spl ; do \
		  echo ; \
		  ./$(BIN) $$i ; \
		done
		@echo

mytests:	all
		@for i in Tests/*.spl ; do \
		  echo ; \
		  echo $$i:; \
		  ./$(BIN) $$i ; \
		  ./$(BIN) --tokens $$i > tokens-my.txt ; \
		  ./Tests/spl-reference --tokens $$i o > tokens-ref.txt ; \
		  diff -q tokens-my.txt tokens-ref.txt ; \
		  rm tokens-my.txt tokens-ref.txt ; \
		  ./$(BIN) --absyn $$i > absyn-my.txt ; \
		  ./Tests/spl-reference --absyn $$i o > absyn-ref.txt ; \
		  diff -q absyn-my.txt absyn-ref.txt ; \
		  rm absyn-my.txt absyn-ref.txt ; \
		done
		@echo

-include depend.mak

depend:		parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o
		rm -f Tests/*~

dist-clean:	clean
		rm -f $(BIN) parser.tab.c parser.tab.h lex.yy.c depend.mak
