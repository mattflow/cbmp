MAIN=tests
GCC_VERSION=10.2.0
IMAGE_NAME=cbmp-primary

default: $(MAIN)

memory: $(MAIN)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -v ./$(MAIN)

test: $(MAIN)
	./$(MAIN)

$(MAIN): $(MAIN).c cbmp.h
	gcc $(MAIN).c -Wall -Wextra -o $(MAIN)

docker-push: docker-build
	docker push mattflow/$(IMAGE_NAME):$(GCC_VERSION)

docker-build:
	docker build \
		-t mattflow/$(IMAGE_NAME):$(GCC_VERSION) \
		--build-arg GCC_VERSION=$(GCC_VERSION) \
		.circleci/images/primary

clean:
	rm -rf $(MAIN)
