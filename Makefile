BISON := bison
FLEX := flex

SOURCES := src/codegen.cpp src/main.cpp
INCLUDES := src/nodes.hh src/parserdecl.h

LLVM_CONFIG := $(shell llvm-config --cxxflags --ldflags --system-libs --libs)
CXXFLAGS = $(LLVM_CONFIG) -fexceptions
GENERATEDINCLUDES := src/parser.tab.hh
GENERATEDSOURCES := src/parser.yy.c src/parser.tab.cc
OUTPUT := bin/ni

.PHONY: default
default: build

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

.PHONY: compile_ll
compile_ll:
	$(OUTPUT) examples/test.ni output.ll

.PHONY: compile_asm
compile_asm:
	llc -filetype=asm output.ll -o output.s

.PHONY: link
link:
	$(CXX) output.s -o a.out

.PHONY: test
test: compile_ll compile_asm link
	./scripts/test.sh

.PHONY: debug
debug: CXXFLAGS += -g -DDEBUG
debug: BISONFLAGS += -t
debug: build
