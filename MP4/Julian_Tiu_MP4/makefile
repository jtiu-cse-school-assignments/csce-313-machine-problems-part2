# makefile

all: dataserver client

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

dataserver: dataserver_624005611.C reqchannel.o 
	g++ -g -o dataserver dataserver_624005611.C reqchannel.o -lpthread #-mmacosx-version-min=10.6

semaphore.o: semaphore.cpp semaphore.H
	g++ -c -g semaphore.cpp
	
BoundedBuffer.o: BoundedBuffer.cpp BoundedBuffer.H semaphore.o
	g++ -c -g BoundedBuffer.cpp semaphore.o
	
client: client_624005611.C reqchannel.o BoundedBuffer.o semaphore.o
	g++ -std=c++11 -g -o client client_624005611.C reqchannel.o BoundedBuffer.o semaphore.o -lpthread
	
clean:
	rm *.o
	rm -f fifo*