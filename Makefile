.PHONY: ni
ni: src/parser.y src/parser.l
	bison -d src/parser.y  -o src/parser.tab.c
	flex -o src/parser.yy.c src/parser.l
	clang src/parser.yy.c src/parser.tab.c src/main.c -o ni

.PHONY: build
build: ni

.PHONY: clean
clean:
	rm -rf ./ni

