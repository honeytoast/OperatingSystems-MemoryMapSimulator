#### Makes 1 executable called memorysimulator
#### Type 'make' in the terminal
#### Type 'make clean'to clean object files after

CC = g++
OPTIONS = -Wall
OBJECTS = main.o process.o

memorysimulator: $(OBJECTS)
	$(CC) $(OPTIONS) $(OBJECTS) -o memorysimulator

main.o: main.cpp
	$(CC) $(OPTIONS) -c main.cpp

process.o: process.cpp
	$(CC) $(OPTIONS) -c process.cpp

clean:
	rm *.o
