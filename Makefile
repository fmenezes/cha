default: build

.PHONY: bison
bison: src/parser.y
	bison -d src/parser.y -b src/parser

.PHONY: flex
flex: src/parser.l
	flex -o src/parser.yy.c src/parser.l

.PHONY: build
build: bison flex
	clang++ src/parser.yy.c src/parser.tab.c src/nodes.cpp src/main.cpp -o ni

.PHONY: clean
clean:
	rm -rf ./ni src/parser.yy.c src/parser.tab.c  src/parser.tab.h
