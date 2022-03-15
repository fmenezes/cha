FROM ubuntu as builder

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && apt-get install build-essential flex bison cmake -y

WORKDIR /app
COPY .clang-format CMakeLists.txt /app/
COPY src src
COPY include include

RUN mkdir build && cd build && cmake .. && make

FROM ubuntu

WORKDIR /app
COPY --from=builder /app/build/ni /app/build/ni
COPY examples examples
COPY scripts scripts

CMD ["./scripts/test.sh"]
