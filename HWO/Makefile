CXX = gcc
CXXFLAGS = 

# Targets
TARGETS = main

# Source files
SRCS = main.c

all: $(TARGETS)

main: main.c
	$(CXX) $(CXXFLAGS) -o main main.c

clean:
	rm -f $(TARGETS)

run: main
	./main

.PHONY: all clean run 