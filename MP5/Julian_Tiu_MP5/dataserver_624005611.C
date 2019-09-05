/* 
    File: dataserver.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2013/06/16

    Dataserver main program for MPs in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "NetworkRequestChannel.H"
#include "reqchannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;
int backlog;
int port_no;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void * connection_handler(void *arg);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(NetworkRequestChannel * _channel, const string & _request) {
  _channel->cwrite("hello to you too");
}

void process_data(NetworkRequestChannel * _channel, const string &  _request) {
  usleep(1000 + (rand() % 5000));
  //_channel.cwrite("here comes data about " + _request.substr(4) + ": " + int2string(random() % 100));
  _channel->cwrite(int2string(rand() % 100));
}

void process_newthread(NetworkRequestChannel * _channel, const string & _request) {
  int error;
  nthreads ++;

  // -- Name new data channel

  string new_channel_name = "data" + int2string(nthreads) + "_";
  //  cout << "new channel name = " << new_channel_name << endl;

  // -- Pass new channel name back to client

  _channel->cwrite(new_channel_name);

  // -- Construct new data channel (pointer to be passed to thread function)
  
  //RequestChannel * data_channel = new RequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);
  NetworkRequestChannel * data_channel = new NetworkRequestChannel(port_no, connection_handler, backlog);

  // -- Create new thread to handle request channel

  pthread_t thread_id;
  //  cout << "starting new thread " << nthreads << endl;
  if (error = pthread_create(& thread_id, NULL, connection_handler, data_channel)) {
    fprintf(stderr, "p_create failed: %s\n", strerror(error));
  }  

}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(NetworkRequestChannel * _channel, const string & _request) {

  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(_channel, _request);
  }
  else if (_request.compare(0, 4, "data") == 0) {
    process_data(_channel, _request);
  }
  else if (_request.compare(0, 9, "newthread") == 0) {
    process_newthread(_channel, _request);
  }
  else {
    _channel->cwrite("unknown request");
  }

}

void * connection_handler(void *arg) {

	cout << "\nEstablished connection. Connection_handler function called." << endl;
	
	NetworkRequestChannel *nrc = (NetworkRequestChannel *)arg;
	
	for(;;) {
		string request = nrc->cread();
		
		if (request.compare("quit") == 0) {
			nrc->cwrite("bye");
			usleep(10000);          // give the other end a bit of time.
			break;                  // break out of the loop;
    	}
		
		process_request(nrc, request);
	}
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

	int c;
	while( (c = getopt(argc, argv, "p:b:")) != -1) {
		switch(c) {
			case 'b':
				backlog = atoi(optarg);
				break;
			case 'p':
				port_no = atoi(optarg);
				break;
			case '?':
				break;
			default:
				break;
		}
	}

  cout << "Establishing control channel... " << flush;
  NetworkRequestChannel control_channel(port_no, connection_handler, backlog); //not8554 later
  cout << "done.\n" << flush;

}

