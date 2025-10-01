FROM ubuntu AS builder
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
RUN apt-get update && apt-get install lsb-release wget software-properties-common gnupg build-essential flex wget bison cmake zlib1g-dev libzstd-dev git -y
RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 21 all && rm llvm.sh
WORKDIR /app
COPY CMakeLists.txt /app/
COPY src src
COPY include include
COPY examples examples
COPY test test
RUN mkdir build && cd build && cmake .. && cmake --build .
RUN cd /app/build && ctest --output-on-failure

FROM ubuntu
RUN apt-get update && apt-get install build-essential -y
COPY --from=builder /app/build/cha /usr/local/bin/cha
ENTRYPOINT [ "/usr/local/bin/cha" ]
CMD [ "--version" ]
