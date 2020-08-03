ORGANIZATION=mattflow
IMAGE_NAME=cbmp-clang

BUILD_COMMAND=docker build -t $(ORGANIZATION)/$(IMAGE_NAME):$@ --build-arg CLANG_VERSION=$@ .
TAG_COMMAND=docker tag $(ORGANIZATION)/$(IMAGE_NAME):$^ $(ORGANIZATION)/$(IMAGE_NAME):$@

push: all
	docker push $(ORGANIZATION)/$(IMAGE_NAME)

all: latest 9 8 6

latest: 9
	$(TAG_COMMAND)

9: 9.0
	$(TAG_COMMAND)

9.0:
	$(BUILD_COMMAND)

8: 8.0
	$(TAG_COMMAND)

8.0:
	$(BUILD_COMMAND)

6: 6.0
	$(TAG_COMMAND)

6.0:
	$(BUILD_COMMAND)