FROM ubuntu

RUN apt-get update && apt-get install build-essential flex bison -y

WORKDIR /app
COPY . .

RUN make

CMD ["make", "test"]
