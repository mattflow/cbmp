memory: build
	valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=1 ./tests

run: build
	./tests

build:
	gcc -o tests tests.c