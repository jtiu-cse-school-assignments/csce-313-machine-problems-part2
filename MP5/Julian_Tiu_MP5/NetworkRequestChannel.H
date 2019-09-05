#ifndef _NetworkRequestchannel_H_
#define _NetworkRequestchannel_H_

#include <iostream>
#include <fstream>
#include <string>

class NetworkRequestChannel {

private:
	unsigned short port_no;
    std::string server_host_name;
	
public:
	int socketfd;
	NetworkRequestChannel(const std::string _server_host_name, const unsigned short _port_no);
	NetworkRequestChannel(int fd);
	NetworkRequestChannel();
	//NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (int *));
	NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (void *), int backlog);
	~NetworkRequestChannel();
	std::string send_request(std::string _request);
	std::string cread();
	int cwrite(std::string _msg);
};

#endif