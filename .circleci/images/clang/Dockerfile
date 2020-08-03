ARG CLANG_VERSION

FROM rsmmr/clang:${CLANG_VERSION}

RUN apt-get update \
    && apt-get install -y \
        valgrind \
        git \
        ssh \
        tar \
        gzip \
        ca-certificates \
    && ln -s $(which clang) /usr/local/bin/gcc

ENTRYPOINT ["/bin/bash"]