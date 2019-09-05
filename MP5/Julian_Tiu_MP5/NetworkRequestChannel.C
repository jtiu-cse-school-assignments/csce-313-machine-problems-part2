#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

#include "NetworkRequestChannel.H"

using namespace std;

const int MAX_MESSAGE = 255;

NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no) {
    //client
    int sockfd, numbytes;  
    char buf[100];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    getaddrinfo((_server_host_name).c_str(), (to_string(_port_no)).c_str(), &hints, &servinfo);
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	
	socketfd = sockfd;
	
}

NetworkRequestChannel::NetworkRequestChannel(int fd) {
	socketfd = fd;
}

//NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (int *)) {
NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (void *), int backlog) {
    //server
    
    port_no = _port_no;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo("localhost", (to_string(_port_no)).c_str(), &hints, &res);
    
    socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(socketfd, res->ai_addr, res->ai_addrlen);
    listen(socketfd, backlog);
    
    addr_size = sizeof their_addr;
    
    while(true) {
    	int new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
    	NetworkRequestChannel * nrc = new NetworkRequestChannel(new_fd);
    	
    	pthread_t * newConnection = new pthread_t();
    	pthread_create(newConnection, NULL, connection_handler, (void *)nrc);
    }
}

NetworkRequestChannel::NetworkRequestChannel() {

}

NetworkRequestChannel::~NetworkRequestChannel() {

}

string NetworkRequestChannel::send_request(string _request) {
    cwrite(_request);
    string s = cread();
    return s;
}

string NetworkRequestChannel::cread() {
    char buf[MAX_MESSAGE];
    
    if (recv(socketfd, buf, MAX_MESSAGE, 0) < 0) {
        perror(string("error").c_str());
    }
    
    string s = buf;
    
    return s;
}

int NetworkRequestChannel::cwrite(string _msg) {
    if (_msg.length() >= MAX_MESSAGE) {
        cerr << "Message too long for Channel!\n";
        return -1;
    }
    
    const char * s = _msg.c_str();
    
    if (send(socketfd, s, strlen(s)+1, 0) < 0) {
        perror(string("error").c_str());
    }
}