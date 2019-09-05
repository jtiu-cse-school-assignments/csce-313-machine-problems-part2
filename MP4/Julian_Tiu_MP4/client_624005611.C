#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <string> 
#include <iostream>

#include "reqchannel.H"
#include "BoundedBuffer.H"
#include "reqchannel.H"

using namespace std; 

Semaphore RTC_lock(1);
Semaphore WTC_lock(1);
int numRequestChannels;
int numDepositToBuffer;
int boundedBufferSize;
int reqThreadCounter = 3;
int workerThreadCounter;
int HISTO_SIZE = 10;

struct requestInfo {
	BoundedBuffer * bb;
	int numDeposits;
	string req;
};

struct eventHandlerInfo {

	BoundedBuffer * bb;
 	map<string, BoundedBuffer*> * statMap;
 	int numberOfRequests;
 	
};

struct statInfo {
 	BoundedBuffer * statBuffer;
 	int * histo;
};

void *depositToBuffer(void *arg) {
	
	requestInfo * parameters = (requestInfo *)arg;
	
	for(int i = 0; i < parameters->numDeposits; i++) {
		parameters->bb->toBuffer(parameters->req);
	}
	
	parameters->bb->toBuffer("DONE");
}

void *eventHandlerFunction(void *arg) {

	bool foreverLoop = true;
	
	eventHandlerInfo * parameters = (eventHandlerInfo *)arg;
	RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
		
    RequestChannel ** RCA = new RequestChannel*[parameters->numberOfRequests];
	for(int i = 0; i < numRequestChannels; i++) {
		string newThread = chan.send_request("newthread");
   	 	RCA[i] = new RequestChannel(newThread, RequestChannel::CLIENT_SIDE);
	}
	
	//chan.send_request("quit");
    map<int, string> fd_to_id;
    
    fd_set readSet;
    FD_ZERO(&readSet);
    fd_set backUp;
    int max = (-1)*99999;
    
     /************************ PRIMING THE PUMP ********************* */
    for(int i = 0; i < parameters->numberOfRequests; i++) {
 		FD_SET(RCA[i]->read_fd(), &readSet);
 		if(RCA[i]->read_fd() > max) {
 			max = RCA[i]->read_fd();
 		}
 		string req = parameters->bb->fromBuffer();
		string name;
		if(req.size() > 5) name = req.substr(5);
        fd_to_id.insert(pair<int,string>(RCA[i]->read_fd(), name));
		RCA[i]->cwrite(req);
 	}
 	
 	backUp = readSet;

	while(foreverLoop) {
 		/*********************** WAIT FOR REPLY ********************* */
 		cout << "\n******** PROCEDURE ********" << endl;
 		int ready;
 		int nready = select(max+1, &readSet, NULL, NULL, NULL);
 		printf("\nNumber of ready fileDes: %i \n", nready);
 		
 		for(int i = 0; i < parameters->numberOfRequests; i++) {
 			if(FD_ISSET(RCA[i]->read_fd(), &readSet)) {
 				ready = RCA[i]->read_fd();
                string response = RCA[i]->cread();
                cout << "Ready file descriptor #: " << ready << endl;
                cout << "response: " << response << endl;
                string key = fd_to_id.find(ready)->second;
                cout << "key: " << key << endl;
        /******************** FORWARD TO STAT BUFFER ***************** */
                BoundedBuffer * theCorrectBuffer = parameters->statMap->find(key)->second;
                theCorrectBuffer->toBuffer(response);
 			}
 		}
 		
 		FD_ZERO(&readSet);
 		for(int i = 0; i < parameters->numberOfRequests; i++) {
			FD_SET(RCA[i]->read_fd(), &readSet);
			if(RCA[i]->read_fd() > max) {
				max = RCA[i]->read_fd();
			}
		}
 		//readSet = backUp;
 		fd_to_id.clear();
 		
 		 /******************** PULL FROM BOUNDED BUFFER ***************** */
 		for(int i = 0; i < parameters->numberOfRequests; i++) {
 			string req = parameters->bb->fromBuffer();
 			cout << "\nnext request: " << req << endl;
			if(req == "DONE") {
 				reqThreadCounter--;
 				cout << "requestThreadCounter: " << reqThreadCounter << endl;
				if(reqThreadCounter == 0) {
                	BoundedBuffer * tempBuff1 = parameters->statMap->find("John Smith")->second;
                	tempBuff1->toBuffer("DONE");
                	BoundedBuffer * tempBuff2 = parameters->statMap->find("Jane Smith")->second;
                	tempBuff2->toBuffer("DONE");
            		BoundedBuffer * tempBuff3 = parameters->statMap->find("Joe Smith")->second;
                	tempBuff3->toBuffer("DONE");
                	
                	for(int i = 0; i < parameters->numberOfRequests; i++) {
                		RCA[i]->cwrite("quit");
                	}
                	
                	foreverLoop = false;
				}
			}
			else if(req.size() > 5) {
			 /*********************** CWRITE REQUEST ********************* */
				string name;
				name = req.substr(5);
				fd_to_id.insert(pair<int,string>(RCA[i]->read_fd(), name));
				RCA[i]->cwrite(req);
			}
 		}
	}
	
	chan.cwrite("quit");
} 

void *statThreadFunction(void *arg) {

	statInfo * parameters = (statInfo*)arg; 
	
	while(true) {
		string dataFromStatBuffer = parameters->statBuffer->fromBuffer();
		
		if(dataFromStatBuffer == "DONE") {
			break;
		}
		else {
			int convertedData = stoi(dataFromStatBuffer);
			int dividedConvertedData = convertedData/10;
			parameters->histo[dividedConvertedData]++;
		}
	}
}

int main(int argc, char * argv[]) {
	
	int c;
	
	while( (c = getopt(argc, argv, "b:w:n:")) != -1) {
		switch(c) {
			case 'b':
				boundedBufferSize = atoi(optarg);
				break;
			case 'w':
				numRequestChannels = atoi(optarg);
				break;
			case 'n':
				numDepositToBuffer = atoi(optarg);
				break;
			case '?':
				break;
			default:
				break;
		}
	}
	
	workerThreadCounter = numRequestChannels;

	pid_t pid = fork();

	if(pid == 0) {
		//child process
		execv("dataserver", NULL);
	}
	else {
		
		BoundedBuffer * requestAndWorkerBuffer = new BoundedBuffer(boundedBufferSize);

		///////////////////////////////////////////////////////////// Request Thread Stuff
	
		requestInfo * reqArguments1 = new requestInfo();
		reqArguments1->bb = requestAndWorkerBuffer;
		reqArguments1->numDeposits = numDepositToBuffer;
		reqArguments1->req += "data John Smith";
	
		requestInfo * reqArguments2 = new requestInfo();
		reqArguments2->bb = requestAndWorkerBuffer;
		reqArguments2->numDeposits = numDepositToBuffer;
		reqArguments2->req += "data Jane Smith";
	
		requestInfo * reqArguments3 = new requestInfo();
		reqArguments3->bb = requestAndWorkerBuffer;
		reqArguments3->numDeposits = numDepositToBuffer;
		reqArguments3->req += "data Joe Smith";
		
		pthread_t requestJohnSmith;
		pthread_t requestJaneSmith;
		pthread_t requestJoeSmith;
	
		pthread_create(&requestJohnSmith, NULL, depositToBuffer, (void *)reqArguments1);
		pthread_join(requestJohnSmith, NULL);
		pthread_create(&requestJaneSmith, NULL, depositToBuffer, (void *)reqArguments2);
		pthread_join(requestJaneSmith, NULL);
		pthread_create(&requestJoeSmith, NULL, depositToBuffer, (void *)reqArguments3);
		pthread_join(requestJoeSmith, NULL);
		
		cout << "\n****** BOUNDED BUFFER ******" << endl;
		requestAndWorkerBuffer->printBuffer();
		
		/////////////////////////////////////////////////////// Event Handler Thread Stuff
	
		BoundedBuffer statisticA(numDepositToBuffer);
		BoundedBuffer statisticB(numDepositToBuffer);
		BoundedBuffer statisticC(numDepositToBuffer);

		map<string, BoundedBuffer*> statisticMap; // we dont want a pointer because we want an object first

		statisticMap.insert(pair<string, BoundedBuffer*>("John Smith", &statisticA));
		statisticMap.insert(pair<string, BoundedBuffer*>("Jane Smith", &statisticB));
		statisticMap.insert(pair<string, BoundedBuffer*>("Joe Smith", &statisticC));

		eventHandlerInfo * eventArguments = new eventHandlerInfo();
		eventArguments->bb = requestAndWorkerBuffer;
		eventArguments->statMap = &statisticMap;
		eventArguments->numberOfRequests = numRequestChannels;
 
		pthread_t eventHandlerThread;
		pthread_create(&eventHandlerThread, NULL, eventHandlerFunction, (void *)eventArguments);
		
		pthread_join(eventHandlerThread, NULL);
		
	
		//////////////////////////////////////////////////////////////// Stat Thread Stuff

		int histoA[HISTO_SIZE];
		int histoB[HISTO_SIZE];
		int histoC[HISTO_SIZE];

		statInfo * statArguments1 = new statInfo();
		statArguments1->statBuffer = &statisticA;
		statArguments1->histo = histoA;

		statInfo * statArguments2 = new statInfo();
		statArguments2->statBuffer = &statisticB;
		statArguments2->histo = histoB;

		statInfo * statArguments3 = new statInfo();
		statArguments3->statBuffer = &statisticC;
		statArguments3->histo = histoC;

		pthread_t statThreadA;
		pthread_t statThreadB;
		pthread_t statThreadC;

		pthread_create(&statThreadA, NULL, statThreadFunction, (void *)statArguments1);
		pthread_join(statThreadA, NULL);
		pthread_create(&statThreadB, NULL, statThreadFunction, (void *)statArguments2);
		pthread_join(statThreadB, NULL);
		pthread_create(&statThreadC, NULL, statThreadFunction, (void *)statArguments3);
		pthread_join(statThreadC, NULL);
	
		cout << "\n*******************************************" << endl;
		cout << "John Smith Histogram: " << endl;
		for(int i = 0; i < sizeof(histoA); i++) {
			cout << histoA[i] << " ";
		}
		cout << "\n*******************************************" << endl;
		cout << "Jane Smith Histogram: " << endl;
		for(int i = 0; i < sizeof(histoB); i++) {
			cout << histoB[i] << " ";
		}
		cout << "\n*******************************************" << endl;
		cout << "Joe Smith Histogram: " << endl;
		for(int i = 0; i < sizeof(histoC); i++) {
			cout << histoC[i] << " ";
		}
		
	}

	return 0;
}