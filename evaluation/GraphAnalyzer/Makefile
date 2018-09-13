CC = g++
INCDIR=include
OBJECTS = Node.o Edge.o Graph.o DeltaGraph.o InOutDeltaGraph.o ABDeltaGraph.o GraphAnalyzer.o
CFLAGS = -Wall -g -std=c++11 -I$(INCDIR)
NAME = graphanalyzer

graphanalyzer: $(OBJECTS)
	$(CC) -o $(NAME) $(OBJECTS)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<

clean:
	rm *.o $(NAME)