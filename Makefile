CC = clang++-12
OPT = -O3
WARNINGS = -Wall -Werror -Wextra -pedantic -Wshadow
STD = -std=c++20
DEBUGFLAGS = -DNDEBUG
BOUNDS_CHECKING =
SAIS_PATH = tpl/libsais
SAIS_STATICLIB = $(SAIS_PATH)/libsais.a
SAIS_INC = $(SAIS_PATH)/src
FLAGS = $(OPT) $(WARNINGS) $(STD) $(DEBUGFLAGS) $(BOUNDS_CHECKING) -I$(INCDIR)

ifeq ($(DEBUG),1)
DEBUGFLAGS := $(filter-out -DNDEBUG, $(DEBUGFLAGS))
OPT = -Og
DEBUGFLAGS = -g
NO_BOUNDS_CHECKING = 0
endif

ifeq ($(NO_BOUNDS_CHECKING),1)
BOUNDS_CHECKING = -DNO_BOUNDS_CHECKING
endif

TESTFLAGS = $(filter-out -DNDEBUG -DNO_BOUNDS_CHECKING,$(FLAGS))

BINDIR = bin
SRCDIR = src
INCDIR = include
DOCSDIR = docs
TARGETS = $(BINDIR)/buildsa $(BINDIR)/querysa

all: $(TARGETS)

$(BINDIR)/buildsa: $(SRCDIR)/buildsa.cc include/parseargs.h include/suffixarray.h include/serial.h $(SAIS_STATICLIB)
	$(CC) $(FLAGS) -I$(SAIS_INC) -o $@ $< $(SAIS_STATICLIB)

$(BINDIR)/querysa: $(SRCDIR)/querysa.cc include/parseargs.h
	$(CC) $(FLAGS) -o $@ $< 

$(SAIS_STATICLIB): $(wildcard $(SAIS_PATH)/src/*.c) $(wildcard $(SAIS_PATH)/src/*.h)
	$(MAKE) -C $(SAIS_PATH) 

$(BINDIR):
	mkdir -p $(BINDIR)

$(DOCSDIR): doc-config
	mkdir -p $(DOCSDIR)
	doxygen doc-config
	$(MAKE) -C $(DOCSDIR)/latex

clean:
	rm -f $(TARGETS)

.PHONY: $(DOCSDIR)
