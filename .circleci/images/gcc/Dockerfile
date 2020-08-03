ARG GCC_VERSION

FROM gcc:${GCC_VERSION}

RUN apt-get update \
    && apt-get install -y \
        valgrind \
        git \
        ssh \
        tar \
        gzip \
        ca-certificates

ENTRYPOINT ["/bin/bash"]