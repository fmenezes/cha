FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && apt-get install build-essential flex bison lsb-release wget software-properties-common -y

RUN cd /tmp && wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 13 clang && rm llvm.sh && apt-get install -y clang-format-13

WORKDIR /app
COPY . .

RUN make

CMD ["make", "test"]
