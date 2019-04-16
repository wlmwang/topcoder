BIN_NAME = a.out

CCFLAG = -g -O0 -Wall -Wshadow -Wno-unused-variable \
		 -Wfloat-equal -Wsign-compare -Wconversion -msse4 -DNDEBUG -std=gnu++0x

CXX = g++

OBJS = main.o

all: clean $(BIN_NAME)

# Just for local testing
check:$(BIN_NAME)
	./$(BIN_NAME) input.10.txt output.10.txt time.txt /tmp/

$(BIN_NAME): $(OBJS)
	$(CXX) $(CCFLAG) -o $@ $^

$(OBJS): %.o: %.cpp
	$(CXX) -c $< -o $@ $(CCFLAG)

clean:
	rm -rf $(BIN_NAME)
	rm -rf $(OBJS)
