FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
ENV LLVM_VERSION=13

RUN apt-get update && apt-get install build-essential flex bison lsb-release wget software-properties-common cmake -y

RUN cd /tmp && wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 13 clang && rm llvm.sh && apt-get install -y clang-format-13

RUN update-alternatives \
    --install /usr/bin/clang             clang            /usr/bin/clang-${LLVM_VERSION} 1 \
    --slave   /usr/bin/clang-cpp         clang-cpp        /usr/bin/clang-cpp-${LLVM_VERSION} && \
    update-alternatives \
    --install /usr/bin/clang++           clang++          /usr/bin/clang++-${LLVM_VERSION} 1 && \
    update-alternatives \
    --install /usr/bin/clang-format      clang-format     /usr/bin/clang-format-${LLVM_VERSION} 1

WORKDIR /app
COPY . .

RUN mkdir build && cd build && cmake .. && make

CMD ["./scripts/test.sh"]
