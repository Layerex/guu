CXX = clang++
CXXFLAGS += -std=c++20 -Wall -Wextra -Wmissing-prototypes -O3
MAIN = main.cpp
OBJS = guu.o
PROG = guu

all: $(PROG)

$(PROG): $(OBJS) $(MAIN)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(MAIN) -o $(PROG)

guu.o: guu.h guu.cpp

clean:
	rm -f guu $(OBJS)

.PHONY: all install clean
