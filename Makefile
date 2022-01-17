LLVM_CONFIG := $(shell llvm-config --cxxflags --ldflags --system-libs --libs)

default: build

.PHONY: bison
bison: src/parser.y
	bison -d src/parser.y -b src/parser

.PHONY: flex
flex: src/parser.l
	flex -o src/parser.yy.c src/parser.l

.PHONY: build
build: bison flex
	clang++ $(LLVM_CONFIG) src/parser.yy.c src/parser.tab.c src/nodes.cpp src/codegen.cpp src/main.cpp -o bin/ni

.PHONY: clean
clean:
	rm -rf bin/ni src/parser.yy.c src/parser.tab.c src/parser.tab.h

.PHONY: test
test:
	./bin/ni < examples/test.ni
