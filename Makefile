LLVM_CONFIG := $(shell llvm-config --cxxflags --ldflags --system-libs --libs)

CXXFLAGS = $(LLVM_CONFIG) -fexceptions -llldCommon -llldDriver -llldMachO2 -llldELF -llldCOFF

ifneq ($(OS),Windows_NT)
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        CXXFLAGS += -lxar
    endif
endif

BISON := bison

FLEX := flex

default: build

.PHONY: bison
bison: src/parser.yy
	$(BISON) $(BISONFLAGS) -d src/parser.yy -b src/parser

.PHONY: flex
flex: src/parser.l
	$(FLEX) $(FLEXFLAGS) -o src/parser.yy.c src/parser.l

.PHONY: ni
ni:
	$(CXX) $(CXXFLAGS) src/parser.yy.c src/parser.tab.cc src/codegen.cpp src/main.cpp -o bin/ni

.PHONY: build
build: clean bison flex ni

.PHONY: clean
clean:
	rm -rf bin/ni src/parser.yy.c src/parser.tab.* a.out output.*

.PHONY: test_ni
test_ni:
	./bin/ni < examples/test.ni

.PHONY: test
test: test_ni
	./scripts/test.sh

.PHONY: debug
debug: CXXFLAGS += -v -g -DDEBUG
debug: BISONFLAGS += -t
debug: build
