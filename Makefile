CC = clang++-12
OPT = -O3
WARNINGS = -Wall -Werror -Wextra -pedantic -Wshadow
STD = -std=c++20
DEBUGFLAGS = -DNDEBUG
BOUNDS_CHECKING =
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
TARGETS = $(BINDIR)/buildsa $(BINDIR)/querysa

all: $(TARGETS)

$(BINDIR)/buildsa: $(SRCDIR)/buildsa.cc include/parseargs.h
	$(CC) $(FLAGS) -o $@ $<

$(BINDIR)/querysa: $(SRCDIR)/querysa.cc include/parseargs.h
	$(CC) $(FLAGS) -o $@ $< 

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f $(TARGETS)
