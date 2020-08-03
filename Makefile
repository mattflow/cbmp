MAIN=tests

default: $(MAIN)

test-full: $(MAIN)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -v ./$(MAIN)

test: $(MAIN)
	./$(MAIN)

$(MAIN): $(MAIN).c cbmp.h
	gcc $(MAIN).c -Wall -Wextra -o $(MAIN)

images:
	cd .circleci/images && make

clean:
	rm -rf $(MAIN)
