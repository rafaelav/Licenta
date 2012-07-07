/*
 * proxy.cpp
 *
 *  Created on: May 29, 2012
 *      Author: rafaela
 */

#include "proxy.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */
#include <stdlib.h>
#include "debug.h"
#include <list>

using namespace std;

int msgCountExt = 0;
int msgCountInt = 0;

typedef struct {
	struct sockaddr_in addr;
	int sock;
} ASSOC;

list<ASSOC> assocList;
fd_set readFDS;

struct sockaddr_in serverAddr;
struct sockaddr_in listenerAddr;
int listenerSock;


// maximum file descriptor of a socket - used in select
int nfds;

void error(const char *msg) {
	perror(msg);
	exit(1);
}

// initializes the server & program
void initialize() {
	dprintf("Initializing server...");

	// open socket
	listenerSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (listenerSock < 0)
		error("ERROR opening socket");

	// setting zeros everywhere
	bzero((char *) &listenerAddr, sizeof(listenerAddr));

	// this is the address used to communicate with incoming connections
	listenerAddr.sin_family = AF_INET; // address family
	listenerAddr.sin_port = htons(LISTEN_PORT); // port number
	listenerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// binding address to socket
	if (bind(listenerSock, (struct sockaddr*) &listenerAddr, sizeof(struct sockaddr)) < 0)
		error("ERROR on binding 1");

	// this is the address used to forward messages to server
	serverAddr.sin_family = AF_INET; // address family
	serverAddr.sin_port = htons(SERVER_PORT); // port number
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// empty readFDS
	FD_ZERO(&readFDS);

	dprintf("Server initialized...");

}

int CreateNewAssociation(struct sockaddr_in sockAddr)
{
	ASSOC newAssoc;
	int newSock;
	struct sockaddr_in newAddr;

	dprintf("Trying to create new association for %s...",inet_ntoa(sockAddr.sin_addr));

	// create new socket
	newSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (newSock < 0)
		error("ERROR opening socket");

	// create new address on localhost from which it will contact inbound machine
	newAddr.sin_family = AF_INET; // address family
	newAddr.sin_port = htons(0); // port number
	newAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// binding address to socket
	if (bind(newSock, (struct sockaddr*) &newAddr, sizeof(struct sockaddr)) < 0)
		error("ERROR on binding 2");

	// keep correspondance
	newAssoc.addr = sockAddr;
	newAssoc.sock = newSock;

	// save in list
	assocList.push_back(newAssoc);

	// update FD_SET
	FD_SET(newSock, &readFDS);
	nfds = newSock;

	return newAssoc.sock;
}

void ReceiveFromExterior()
{
	int n;
	int size;
	bool val;
	char msg[MSG_SIZE];
	struct sockaddr_in sockAddr, addr;
	int intSock;
	int addrLen = sizeof(sockAddr);
	bool found = false;
	int extraInfo = 0;	// used for other info received from modules

	memset(msg, 0, MSG_SIZE);
	dprintf("Trying to receive message from exterior...");

	// receive message
	n = recvfrom(listenerSock, msg, MSG_SIZE, 0, (struct sockaddr *) &sockAddr, (socklen_t *) &addrLen);
	size = n;
	if(n < 0)
	{
		error("ReceiveFromExterior - Problem with receiving from socket");
	}

	printf("[REC EXT->*]: from %s:UDP%u : %d \n",
		inet_ntoa(sockAddr.sin_addr),
		ntohs(sockAddr.sin_port),n); // network_to_host_short for port

	// any manipulation of packets will be done just after the communication through the proxy will be established
	msgCountExt++;
	if(msgCountExt>=MAX_MSG_INIT)
	{
		msgCountExt = MAX_MSG_INIT+1;
	}

	// just after the first MAX_MSG_INIT messages have been exchanged
	if(msgCountExt>=MAX_MSG_INIT)
	{
		// function for manipulating message before sending the message to the interior machine
		val = ProcessMessageFromExterior(msg,size,sockAddr,extraInfo);
		dprintf("[EXT] extraInfo field -> %d", extraInfo);
		if(val == false)
		{
			dprintf("Not forwarding message from exterior to interior");
			return;
		}
	}
	// verifying if socket corresponding to this client on server side exists
	list<ASSOC>::iterator it;

	for(it=assocList.begin(); it!=assocList.end(); it++)
	{
		addr=it->addr;

		// check if it is the address from which we have received a message
		if(addr.sin_addr.s_addr == sockAddr.sin_addr.s_addr && addr.sin_family == sockAddr.sin_family && addr.sin_port == sockAddr.sin_port)
		{
			intSock = it->sock;

			found = true;
			break;
		}
	}

	// if NOT -> create socket + correspondence (ASSOC)+ update FD_SET
	if(found == false)
	{
		intSock = CreateNewAssociation(sockAddr);
	}

	dprintf("Trying to send message to interior...");

	n = sendto(intSock, msg, size+extraInfo, 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	if(n < 0)
	{
		error("ReceiveFromExterior - Problem with sending to socket");
	}


	printf("[SEN *->INT]: to %s:UDP%u : %d \n",
		inet_ntoa(serverAddr.sin_addr),
		ntohs(serverAddr.sin_port),n); // network_to_host_short for port
}

void ReceiveFromInterior(ASSOC assoc)
{
	char msg[MSG_SIZE];
	int size;
	int n;
	bool val;	// used to analyze if message will be forwarded or not
	struct sockaddr_in addr;
	int addrLen = sizeof(addr);
	int extraInfo = 0;	// used for other info received from modules

	dprintf("Trying to RECEIVE message from interior...");

	// receive message from inbound machine
	n = recvfrom(assoc.sock, msg, MSG_SIZE, 0, (struct sockaddr *) &addr, (socklen_t *) &addrLen);
	size = n;
	if(n < 0)
	{
		error("ReceiveFromInterior - Problem with receiving from socket");
	}

	printf("[REC INT->*]: from %s:UDP%u : %d \n",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port),size); // network_to_host_short for port

	// any manipulation of packets will be done just after the communication through the proxy will be established
	msgCountInt++;
	if(msgCountInt>=MAX_MSG_INIT)
	{
		msgCountInt = MAX_MSG_INIT+1;
	}

	// just after the first MAX_MSG_INIT messages have been exchanged
	if(msgCountInt>=MAX_MSG_INIT)
	{
		// function for manipulating message before sending the message to the exterior world
		val = ProcessMessageFromInterior(msg,size,addr,extraInfo);
		dprintf("[EXT] extraInfo field -> %d", extraInfo);
		if(val == false)
		{
			dprintf("Not forwarding message from interior to exterior");
			return;
		}
	}

	// send to exterior knowing the correspondence because of association (assoc)
	dprintf("Trying to SEND message to exterior ...");
	n = sendto(listenerSock, msg, size+extraInfo, 0, (struct sockaddr *) &assoc.addr, sizeof(assoc.addr));
	if(n < 0)
	{
		error("ReceiveFromInterior - Problem with sending to socket");
	}

	printf("[SEN *->EXT]: to %s:UDP%u : %d \n",
		inet_ntoa(assoc.addr.sin_addr),
		ntohs(assoc.addr.sin_port),n); // network_to_host_short for port

}

int main(int argc, char** argv) {

	init(argc,argv);
	initialize();

	// add socket to the reading file descriptors vector -> select will be able to trigger the socket
	FD_SET(listenerSock, &readFDS);
	nfds = listenerSock;

	// used for backup because select will modify them
	fd_set readFDS_Temp;
	int crtSock;

	while (1) {
		// making backup
		readFDS_Temp = readFDS;

		// call select -> will listen to readFDS and see if anything is available
		if (select(nfds + 1, &readFDS_Temp, NULL, NULL, NULL) == -1)
			error("ERROR in select");

		// CLIENT ---> on listenerSock
		if(FD_ISSET(listenerSock,&readFDS_Temp))
		{
			ReceiveFromExterior();
		}

		// go throw all descriptors to see if any of them is set -> means it can be read from it
		list<ASSOC>::iterator it;
		
		for(it=assocList.begin(); it!=assocList.end(); it++)
		{
			crtSock=it->sock;

			// nothing received through this socket
			if(!FD_ISSET(crtSock,&readFDS_Temp))
				continue;

			// there is something to read from socket
			ReceiveFromInterior(*it);
		}

	}
	close(listenerSock);
	return 0;
}
