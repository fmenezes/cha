BISON := bison
FLEX := flex
FORMAT := clang-format

SOURCES := src/codegen.cpp src/asmcodegen.cpp src/main.cpp
INCLUDES := src/nodes.hh src/parserdecl.h

CXXFLAGS = -std=c++14 -stdlib=libc++
GENERATEDINCLUDES := src/parser.tab.hh
GENERATEDSOURCES := src/parser.yy.c src/parser.tab.cc
OUTPUT := bin/ni

.PHONY: default
default: build

.PHONY: git-hooks
git-hooks:
	rm -rf "$$(pwd)/.git/hooks"
	ln -s "$$(pwd)/.githooks" "$$(pwd)/.git/hooks"

.PHONY: check-format
check-format:
	$(FORMAT) -n --Werror -i $(INCLUDES) $(SOURCES)

.PHONY: format
format:
	$(FORMAT) -i $(INCLUDES) $(SOURCES)

.PHONY: bison
bison: src/parser.yy
	$(BISON) $(BISONFLAGS) -d src/parser.yy -b src/parser

.PHONY: flex
flex: src/parser.l
	$(FLEX) $(FLEXFLAGS) -o src/parser.yy.c src/parser.l

.PHONY: test_ld
test_ld:
	$(CXX) output.o -o a.out

.PHONY: ni
ni:
	$(CXX) $(CXXFLAGS) $(GENERATEDSOURCES) $(SOURCES) -o $(OUTPUT)

.PHONY: build
build: clean bison flex ni

.PHONY: clean
clean:
	rm -rf $(OUTPUT) $(GENERATEDINCLUDES) $(GENERATEDSOURCES) a.out output.*

.PHONY: link
link:
	$(CXX) output.s -o a.out

.PHONY: compile_asm
compile_asm:
	$(OUTPUT) -asm examples/test.ni output.s

.PHONY: test
test: compile_asm link
	./scripts/test.sh

.PHONY: debug
debug: CXXFLAGS += -g -DDEBUG
debug: BISONFLAGS += -t
debug: build
