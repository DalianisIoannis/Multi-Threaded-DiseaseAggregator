CC = gcc
CFLAGS = -g3 -Wall
LDFLAGS = -lm -lpthread

ODIR = build
IDIR = headers
SDIR = src

EXECUTABLE = master

_DEPS = general.h workers.h countryList.h pipes.h fatherFunctions.h statistics.h patients.h linkedList.h statistics.h HashTable.h AVL.h MaxHeap.h signals.h ServerClient.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = master.o workers.o countryList.o pipes.o fatherFunctions.o statistics.o patients.o linkedList.o general.o statistics.o HashTable.o AVL.o MaxHeap.o signals.o ServerClient.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(ODIR)/*.o
	rm -f $(EXECUTABLE)
	rm -f whoServer
	rm -f whoClient


all: $(EXECUTABLE)

valgrind:
	valgrind --track-origins=yes --trace-children=yes --leak-check=full ./master -w 5 -b 32 -s "127.0.0.1" -p 9003 -i "./input_dir/"

run:
	./master -w 5 -b 32 -s 10 -p 15 -i "./input_dir/"	

tcp:
	$(CC) $(CFLAGS) -c $(SDIR)/threadQueue.c -o $(ODIR)/threadQueue.o $(LDFLAGS)
	$(CC) $(CFLAGS) -c $(SDIR)/ServerClient.c -o $(ODIR)/ServerClient.o $(LDFLAGS)
	$(CC) $(CFLAGS) -c $(SDIR)/whoServer.c -o $(ODIR)/whoServer.o $(LDFLAGS)
	$(CC) $(CFLAGS) -c $(SDIR)/whoClient.c -o $(ODIR)/whoClient.o $(LDFLAGS)
	$(CC) $(CFLAGS) $(ODIR)/whoClient.o $(ODIR)/ServerClient.o $(ODIR)/threadQueue.o -o whoClient $(LDFLAGS)
	$(CC) $(CFLAGS) $(ODIR)/whoServer.o $(ODIR)/ServerClient.o $(ODIR)/statistics.o $(ODIR)/threadQueue.o -o whoServer $(LDFLAGS)

Server:
	valgrind --track-origins=yes --trace-children=yes --leak-check=full ./whoServer -q 9002 -s 9003 -w 5 -b 32

Client:
	valgrind --track-origins=yes --trace-children=yes --leak-check=full ./whoClient -q queryFile -w numThreads -sp servPort -sip servIP