ARG debian_version="bullseye"
ARG base_image="debian:${debian_version}-slim"

FROM ${base_image} 

RUN DEBIAN_FRONTEND=noninteractive \
  apt-get update \
  && apt-get install --yes --no-install-recommends \
    valgrind