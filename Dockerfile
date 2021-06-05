FROM ubuntu:20.10
LABEL maintainer="njeka0108@gmail.com"
ENV container=docker
ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    sed -i 's/# deb/deb/g' /etc/apt/sources.list && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update -qq && \
    apt-get install -yq --no-install-recommends \
    gcc gdb make \
    zlib1g-dev \
    protobuf-c-compiler libprotobuf-c-dev \
    python3 python3-pip python3-venv python3-dev python3-setuptools && \
    apt-get clean && \
    rm -rf /usr/share/doc/* /usr/share/man/* /var/lib/apt/lists/* /tmp/* /var/tmp/*

WORKDIR /opt/protobuf_serializer
