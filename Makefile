BISON := bison
FLEX := flex
FORMAT := clang-format

SOURCES := src/ast/ast.cpp src/codegen/codegen.cpp src/codegen/asmcodegen.cpp src/main.cpp
INCLUDES := include/ast/ast.hh include/codegen/codegen.hh include/ast/parserdecl.h

CXXFLAGS = -std=c++14 -I generated -I include
GENERATEDINCLUDES := generated/parser.tab.hh
GENERATEDSOURCES := generated/parser.yy.c generated/parser.tab.cc
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
bison: src/ast/parser.yy
	$(BISON) $(BISONFLAGS) -d src/ast/parser.yy -b generated/parser

.PHONY: flex
flex: src/ast/parser.l
	$(FLEX) $(FLEXFLAGS) -o generated/parser.yy.c src/ast/parser.l

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
