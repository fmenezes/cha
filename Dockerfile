FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && apt-get install build-essential flex bison cmake libgtest-dev -y

WORKDIR /app
COPY .clang-format CMakeLists.txt /app/
COPY src src
COPY include include
COPY examples examples
COPY scripts scripts
COPY test test

RUN mkdir build && cd build && cmake .. && cmake --build .

CMD ["bash", "-c", "cd /app/build && ctest --output-on-failure && cd /app && ./scripts/test.sh"]
