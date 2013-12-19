CC = gcc
CFLAGS = -Wall -Wextra -g -pedantic
LDFLAGS = -pthread
OBJS = main.o mergesort.o
OUTPUT = mt_sort
CHK_SOURCES = main.c mergesort.h mergesort.c

mt_sort: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(OUTPUT)

main.o: mergesort.h
mergesort.o: mergesort.h

.PHONY: clean
clean:
	rm -rf $(OUTPUT) $(OBJS) .~

.PHONY: check-syntax
check-syntax:
	$(CC) $(CFLAGS) -fsyntax-only $(CHK_SOURCES)
