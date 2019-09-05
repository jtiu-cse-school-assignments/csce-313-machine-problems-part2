# makefile

all: dataserver client

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

dataserver: dataserver.C reqchannel.o 
	g++ -g -o dataserver dataserver.C reqchannel.o -lpthread

semaphore.o: semaphore.cpp semaphore.H
	g++ -c -g semaphore.cpp
	
BoundedBuffer.o: BoundedBuffer.cpp BoundedBuffer.H semaphore.o
	g++ -c -g BoundedBuffer.cpp semaphore.o
	
client: client.C reqchannel.o BoundedBuffer.o semaphore.o
	g++ -g -o client client.C reqchannel.o BoundedBuffer.o semaphore.o
	
clean:
	#rm *.o
	rm -f fifo*