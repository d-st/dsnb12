#
# Makefile for SPL compiler
#

CC = gcc
CFLAGS = -Wall -Wunused -g
LDFLAGS = -g
LDLIBS = -lm

SRCS = main.c utils.c sym.c absyn.c parser.tab.c lex.yy.c semant.c table.c types.c varalloc.c codegen.c
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

check:	all
		@for i in Tests/*.spl ; do \
		  echo $$i:; \
		  echo -n " - testing clean parsing: "; \
		  ./$(BIN) $$i o >/dev/null && echo "ok" || echo "failed"; \
		  echo -n " - reference testing tokens: "; \
		  ./$(BIN) --tokens $$i o > tokens-my.txt ; \
		  ./Tests/spl-reference --tokens $$i o > tokens-ref.txt ; \
		  diff -q tokens-my.txt tokens-ref.txt>/dev/null && echo "ok" || echo "failed"; \
		  rm tokens-my.txt tokens-ref.txt ; \
		  echo -n " - reference testing abstract syntax: "; \
		  ./$(BIN) --absyn $$i o > absyn-my.txt ; \
		  ./Tests/spl-reference --absyn $$i o > absyn-ref.txt ; \
		  diff -q absyn-my.txt absyn-ref.txt>/dev/null && echo "ok" || echo "failed"; \
		  rm absyn-my.txt absyn-ref.txt ; \
		  echo -n " - reference testing symbol tables: "; \
		  ./$(BIN) --tables $$i o | ./Tests/sort-table-output.pl > tables-my.txt ; \
		  ./Tests/spl-reference --tables $$i o | ./Tests/sort-table-output.pl > tables-ref.txt ; \
		  diff -q tables-my.txt tables-ref.txt>/dev/null && echo "ok" || echo "failed"; \
		  rm -f tables-my.txt tables-ref.txt o; \
		  echo -n " - reference testing variable allocation: "; \
		  ./$(BIN) --vars $$i o | ./Tests/sort-vars-output.pl > vars-my.txt ; \
		  ./Tests/spl-reference --vars $$i o | ./Tests/sort-vars-output.pl > vars-ref.txt ; \
		  diff -q vars-my.txt vars-ref.txt>/dev/null && echo "ok" || echo "failed"; \
		  rm -f vars-my.txt vars-ref.txt o; \
		  echo -n " - comparing assembler output: "; \
		  ./$(BIN) $$i o-my.txt ; \
		  ./Tests/spl-reference $$i o-ref.txt ; \
		  diff -q o-my.txt o-ref.txt>/dev/null && echo "ok" || echo "failed"; \
		  rm -f o-my.txt o-ref.txt; \
		done
		@echo

check-errors:	all
		@for i in TestsErrors/*.spl ; do \
		  echo -n $$i:; \
		  ./$(BIN) $$i o > errors-my.txt ; \
		  ./Tests/spl-reference $$i o > errors-ref.txt ; \
		  diff -q errors-my.txt errors-ref.txt>/dev/null && echo " ok" || (echo " failed"; \
		  ./$(BIN) $$i; o \
		  ./Tests/spl-reference $$i o); \
		  rm errors-my.txt errors-ref.txt; \
		done
		@echo

-include depend.mak

depend:		parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o o
		rm -f Tests/*~ Tests/o

dist-clean:	clean
		rm -f $(BIN) parser.tab.c parser.tab.h lex.yy.c depend.mak
