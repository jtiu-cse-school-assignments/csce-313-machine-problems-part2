#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <map>

#include "reqchannel.H"
#include "BoundedBuffer.H"
#include "reqchannel.H"

using namespace std; 

Semaphore RTC_lock(1);
Semaphore WTC_lock(1);
int numWorkerThreads;
int numDepositToBuffer;
int reqThreadCounter = 3;
int workerThreadCounter = numWorkerThreads;
int HISTO_SIZE = 10;

struct requestInfo {
	BoundedBuffer * bb;
	int numDeposits;
	string req;
};

struct workerInfo {
	BoundedBuffer * bb;
	map<string, BoundedBuffer*> * statMap;
	RequestChannel * reqChannel;
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

void *workerThreadFunction(void *arg) {
	
	workerInfo * parameters = (workerInfo *)arg;
	
	while(true) {
		// pull from bounded buffer
		string sendInfo = parameters->bb->fromBuffer();
		if(sendInfo == "DONE") {
			RTC_lock.P();
			reqThreadCounter--;
			if(reqThreadCounter == 0) {
				for(int i = 0; i < numWorkerThreads; i++) {
					//send quit for all worker threads
					parameters->bb->toBuffer("quit");
				}
				//break;
			}
			RTC_lock.V();
		}
		else if(sendInfo == "quit") {
			WTC_lock.P();
			
			workerThreadCounter--;
			
			if(workerThreadCounter == 0) {
				//send quit to all stat buffer
                BoundedBuffer * tempBuff1 = parameters->statMap->find("John Smith")->second;
                tempBuff1->toBuffer("DONE");
                
                BoundedBuffer * tempBuff2 = parameters->statMap->find("Jane Smith")->second;
                tempBuff2->toBuffer("DONE");
            	
            	BoundedBuffer * tempBuff3 = parameters->statMap->find("Joe Smith")->second;
                tempBuff3->toBuffer("DONE");
                
			}
			
			WTC_lock.V();
			
			parameters->reqChannel->send_request("quit");
			
			break;
		}
		else {
			// send request
			cout << "\nsendInfo: " << sendInfo << endl;
			string response = parameters->reqChannel->send_request(sendInfo);
			cout << "\nresponse: " << response << endl;
			// when request is sent back, put reply to statistical buffer
				// substring info to get the name of the reply. This reply is the key to the map
				// then map will return a buffer. toBuffer(response) to that buffer.
			//string name = sendInfo.substr(5);
			//BoundedBuffer * statMap2 = parameters->statMap->find(name)->second;
			//statMap2->toBuffer(response);
		}
	}
}

void *statThreadFunction(void *arg) {

// 	statInfo * parameters = (statInfo*)arg; 
// 	
// 	while(true) {
// 		string dataFromStatBuffer = parameters->statBuffer->fromBuffer();
// 		
// 		if(dataFromStatBuffer == "DONE") {
// 			break;
// 		}
// 		int convertedData = stoi(dataFromStatBuffer);
// 		//divide this data by 10 to get the proper index in the histogram
// 		int dividedConvertedData = convertedData/10;
// 		//increment the data in that histogram index by 1
// 		parameters->histo[dividedConvertedData]++;
// 		
// 	}
}

int main(int argc, char * argv[]) {

	int c;
	
	while( (c = getopt(argc, argv, "w:n:")) != -1) {
		switch(c) {
			case 'w':
				numWorkerThreads = atoi(optarg);
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

// 	pid_t pid = fork();
// 
// 	if(pid == 0) {
// 		// child process
// 		execv("dataserver", NULL);
// 	}
// 	else {
		cout << "CLIENT STARTED:" << endl;

		cout << "Establishing control channel... " << flush;
		RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
		cout << "done." << endl;
	
		BoundedBuffer requestAndWorkerBuffer(numWorkerThreads);

		///////////////////////////////////////////////////////////////// Request Thread Stuff

		pthread_t requestJohnSmith;
		pthread_t requestJaneSmith;
		pthread_t requestJoeSmith;
	
		requestInfo reqArguments1;
		reqArguments1.bb = &requestAndWorkerBuffer;
		reqArguments1.numDeposits = numDepositToBuffer;
		reqArguments1.req += "data John Smith";
	
		requestInfo reqArguments2;
		reqArguments2.bb = &requestAndWorkerBuffer;
		reqArguments1.numDeposits = numDepositToBuffer;
		reqArguments2.req += "data Jane Smith";
	
		requestInfo reqArguments3;
		reqArguments3.bb = &requestAndWorkerBuffer;
		reqArguments1.numDeposits = numDepositToBuffer;
		reqArguments3.req += "data Joe Smith";
	
		pthread_create(&requestJohnSmith, NULL, depositToBuffer, (void *)&reqArguments1);
		pthread_create(&requestJaneSmith, NULL, depositToBuffer, (void *)&reqArguments2);
		pthread_create(&requestJoeSmith, NULL, depositToBuffer, (void *)&reqArguments3);
	
		////////////////////////////////////////////////////////////////// Worker Thread Stuff
	
		BoundedBuffer statisticA(numDepositToBuffer);
		BoundedBuffer statisticB(numDepositToBuffer);
		BoundedBuffer statisticC(numDepositToBuffer);
	
		map<string, BoundedBuffer*> statisticMap; // we dont want a pointer because we want an object first
	
		statisticMap.insert(pair<string, BoundedBuffer*>("John Smith", &statisticA));
		statisticMap.insert(pair<string, BoundedBuffer*>("Jane Smith", &statisticB));
		statisticMap.insert(pair<string, BoundedBuffer*>("Joe Smith", &statisticC));
		
		pthread_t workerThreadIDs[numWorkerThreads];
		workerInfo workerInfoArray[numWorkerThreads];
		
		for(int i = 0; i < numWorkerThreads; i++) {
		
			workerInfoArray[i].bb = &requestAndWorkerBuffer;
			workerInfoArray[i].statMap = &statisticMap;
			string newThread = chan.send_request("newthread");
			workerInfoArray[i].reqChannel = new RequestChannel(newThread, RequestChannel::CLIENT_SIDE);
			pthread_create(&workerThreadIDs[i], NULL, workerThreadFunction, (void *)&workerInfoArray[i]);
		}
	
		//////////////////////////////////////////////////////////////////// Stat Thread Stuff
	
		int histoA[HISTO_SIZE];
		int histoB[HISTO_SIZE];
		int histoC[HISTO_SIZE];
	
		pthread_t statThreadA;
		pthread_t statThreadB;
		pthread_t statThreadC;
	
		statInfo statArguments1;
		statArguments1.statBuffer = &statisticA;
		statArguments1.histo = histoA;
	
		statInfo statArguments2;
		statArguments2.statBuffer = &statisticB;
		statArguments2.histo = histoB;
	
		statInfo statArguments3;
		statArguments3.statBuffer = &statisticC;
		statArguments3.histo = histoC;
	
		pthread_create(&statThreadA, NULL, statThreadFunction, (void *)&statArguments1);
		pthread_create(&statThreadB, NULL, statThreadFunction, (void *)&statArguments2);
		pthread_create(&statThreadC, NULL, statThreadFunction, (void *)&statArguments3);

		// main needs to wait for statthreads to finish, so join here for stat
		pthread_join(statThreadA, NULL);
		pthread_join(statThreadB, NULL);
		pthread_join(statThreadC, NULL);
		
// 		cout << "\n*******************************************" << endl;
// 		cout << "John Smith Histogram: " << endl;
// 		for(int i = 0; i < sizeof(histoA); i++) {
// 			cout << histoA[i] << " ";
// 		}
// 		cout << "\n*******************************************" << endl;
// 		cout << "Jane Smith Histogram: " << endl;
// 		for(int i = 0; i < sizeof(histoB); i++) {
// 			cout << histoB[i] << " ";
// 		}
// 		cout << "\n*******************************************" << endl;
// 		cout << "Joe Smith Histogram: " << endl;
// 		for(int i = 0; i < sizeof(histoC); i++) {
// 			cout << histoC[i] << " ";
// 		}
	
		//close ALL channels here
		chan.send_request("quit");
	//}

	return 0;
}