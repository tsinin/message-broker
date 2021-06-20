FROM ubuntu:latest

COPY ./ ./

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update -y \
 && apt-get install -y cmake \
 && apt-get install -y make \
 && apt-get install -y libboost-all-dev \
 && apt-get install -y libcereal-dev \
 && apt-get install -y libyaml-cpp-dev \
 && apt-get install -y libgtest-dev \
 && apt-get install -y build-essential

RUN mkdir build \
 && cd build \
 && cmake .. \
 && make


EXPOSE 9090

ENTRYPOINT [ "./build/server", "server_config.yaml" ]