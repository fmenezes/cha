LLVM_CONFIG := $(shell llvm-config --cxxflags --ldflags --system-libs --libs)

default: build

.PHONY: bison
bison: src/parser.yy
	bison -d src/parser.yy -b src/parser

.PHONY: flex
flex: src/parser.l
	flex -o src/parser.yy.c src/parser.l

.PHONY: test_ld
test_ld:
	ld output.o -lSystem -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib

.PHONY: clang
clang:
	clang++ $(LLVM_CONFIG) src/parser.yy.c src/parser.tab.cc src/codegen.cpp src/main.cpp -o bin/ni

.PHONY: build
build: bison flex clang 

.PHONY: clean
clean:
	rm -rf bin/ni src/parser.yy.c src/parser.tab.* a.out output.*

.PHONY: test_ni
test_ni:
	./bin/ni < examples/test.ni

.PHONY: test
test: test_ni test_ld