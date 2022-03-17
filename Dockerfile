FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && apt-get install build-essential flex bison cmake -y

WORKDIR /app
COPY .clang-format CMakeLists.txt /app/
COPY src src
COPY include include
COPY examples examples
COPY scripts scripts

RUN mkdir build && cd build && cmake .. && make

CMD ["./scripts/test.sh"]
