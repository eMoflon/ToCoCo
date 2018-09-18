CC = g++
INCDIR=include
OBJECTS = Node.o Edge.o Graph.o DeltaGraph.o InOutDeltaGraph.o ABDeltaGraph.o GraphAnalyzer.o
CFLAGS = -Wall -g -std=c++11 -I$(INCDIR)
NAME = graphanalyzer.exe

.PHONY: all clean

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<

clean:
	rm *.o $(NAME)