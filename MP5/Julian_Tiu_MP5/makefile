# makefile

all: NetworkRequestChannel.o client dataserver 

NetworkRequestChannel.o: NetworkRequestChannel.H NetworkRequestChannel.C
	g++ -std=c++11 -c -g NetworkRequestChannel.C -lpthread

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

dataserver: dataserver_624005611.C reqchannel.o  NetworkRequestChannel.o
	g++ -g -o dataserver dataserver_624005611.C reqchannel.o NetworkRequestChannel.o -lpthread #-mmacosx-version-min=10.6

semaphore.o: semaphore.cpp semaphore.H
	g++ -c -g semaphore.cpp
	
BoundedBuffer.o: BoundedBuffer.cpp BoundedBuffer.H semaphore.o
	g++ -c -g BoundedBuffer.cpp semaphore.o
	
client: client_624005611.C reqchannel.o BoundedBuffer.o semaphore.o NetworkRequestChannel.o
	g++ -std=c++11 -g -o client client_624005611.C reqchannel.o BoundedBuffer.o semaphore.o NetworkRequestChannel.o -lpthread
	
clean:
	rm *.o
	rm -f fifo*