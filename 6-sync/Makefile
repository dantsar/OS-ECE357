CC=gcc
CFLAGS=-ggdb -std=gnu99

all: tSpin tSem tFifo

tas64.o: tas64.S tas.h
	$(CC) $(CFLAGS) tas64.S -c

spinlock.o: spinlock.c spinlock.h
	$(CC) $(CFLAGS) spinlock.c -c

tSpin: spinlock.o tas64.o tSpin.c
	$(CC) $(CFLAGS) -o $@ $^ 

sem.o: sem.c sem.h
	$(CC) $(CFLAGS) sem.c -c

tSem: tsem.c sem.o spinlock.o tas64.o
	$(CC) $(CFLAGS) -o $@ $^

fifo.o: fifo.c fifo.h
	$(CC) $(CFLAGS) fifo.c -c

tFifo: tfifo.c fifo.o spinlock.o sem.o tas64.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@echo "removing object files"
	@rm *.o
	@echo "Done!"
	
remove:
	@echo "removing executables"
	@rm tSem tSpin tFifo
	@echo "Done!"
