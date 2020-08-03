MAIN=tests

default: $(MAIN)

memory: $(MAIN)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -v ./$(MAIN)

test: $(MAIN)
	./$(MAIN)

$(MAIN): $(MAIN).c cbmp.h
	gcc $(MAIN).c -Wall -Wextra -o $(MAIN)

clean:
	rm -rf $(MAIN)
