FROM ubuntu:latest

COPY ./ ./

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update -y \
 && apt-get install -y cmake \
                       make \
                       libboost-all-dev \
                       libcereal-dev \
                       libyaml-cpp-dev \
                       libgtest-dev \
                       build-essential

RUN mkdir build \
 && cd build \
 && cmake .. \
 && make


EXPOSE 9090

ENTRYPOINT [ "./build/server", "server_config.yaml" ]