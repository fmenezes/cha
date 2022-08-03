FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && apt-get install lsb-release wget software-properties-common gnupg build-essential flex wget bison cmake zlib1g-dev -y && wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 14 all && rm llvm.sh

WORKDIR /app
COPY .clang-format CMakeLists.txt /app/
COPY src src
COPY include include
COPY examples examples
COPY scripts scripts
COPY test test

RUN mkdir build && cd build && cmake .. && cmake --build .

CMD ["bash", "-c", "cd /app/build && ctest --output-on-failure && cd /app && ./scripts/test.sh"]
