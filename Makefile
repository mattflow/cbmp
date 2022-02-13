default: tests

test: tests
	./tests

tests: tests.c
	gcc -Wall -o tests tests.c

clean:
	rm -rf tests