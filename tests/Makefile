# Build and run the security regression tests.
#   make        - build and run with AddressSanitizer + UBSan
#   make clean  - remove the built binary and test artifacts

CC      ?= cc
CFLAGS  ?= -Wall -Wextra -O1 -g -fsanitize=address,undefined

test: test_security
	./test_security

test_security: test_security.c ../cbmp.c ../cbmp.h
	$(CC) $(CFLAGS) ../cbmp.c test_security.c -o test_security

clean:
	rm -f test_security *.bmp

.PHONY: test clean
