CXX = gcc
CXXFLAGS = 

# Targets
TARGETS = main stack

# Source files
SRCS = main.c

all: $(TARGETS)

main: main.c
	$(CXX) $(CXXFLAGS) -o main main.c

stack: stack.c
	$(CXX) $(CXXFLAGS) -o stack stack.c

clean:
	rm -f $(TARGETS)

run_main: main
	./main
	
run_stack: stack
	./stack

.PHONY: all clean run_main run_stack