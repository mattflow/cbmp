MAIN=tests

default: $(MAIN)

test: $(MAIN)
	./$(MAIN)

$(MAIN): $(MAIN).c cbmp.h
	gcc $(MAIN).c -Wall -Wextra -o $(MAIN)

clean:
	rm -rf $(MAIN)
