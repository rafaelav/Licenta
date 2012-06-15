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
	bool val;
	char msg[MSG_SIZE];
	struct sockaddr_in sockAddr, addr;
	int intSock;
	int addrLen = sizeof(sockAddr);
	bool found = false;

	dprintf("Trying to receive message from exterior...");

	// receive message
	n = recvfrom(listenerSock, msg, MSG_SIZE, 0, (struct sockaddr *) &sockAddr, (socklen_t *) &addrLen);
	if(n < 0)
	{
		error("ReceiveFromExterior - Problem with receiving from socket");
	}

	printf("[REC EXT->*]: from %s:UDP%u : %d \n",
		inet_ntoa(sockAddr.sin_addr),
		ntohs(sockAddr.sin_port),n); // network_to_host_short for port

	// function for manipulating message before sending the message to the interior machine
	val = ProcessMessageFromExterior(msg,n,sockAddr);
	if(val == false)
		return;

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

	n = sendto(intSock, msg, strlen(msg)+1, 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	if(n < 0)
	{
		error("ReceiveFromExterior - Problem with sending to socket");
	}


	printf("[SEN *->INT]: to %s:UDP%u : %d \n",
		inet_ntoa(serverAddr.sin_addr),
		ntohs(serverAddr.sin_port),strlen(msg)); // network_to_host_short for port
}

void ReceiveFromInterior(ASSOC assoc)
{
	char msg[MSG_SIZE];
	int n;
	bool val;	// used to analyze if message will be forwarded or not
	struct sockaddr_in addr;
	int addrLen = sizeof(addr);

	dprintf("Trying to RECEIVE message from interior...");

	// receive message from inbound machine
	n = recvfrom(assoc.sock, msg, MSG_SIZE, 0, (struct sockaddr *) &addr, (socklen_t *) &addrLen);
	if(n < 0)
	{
		error("ReceiveFromInterior - Problem with receiving from socket");
	}

	printf("[REC INT->*]: from %s:UDP%u : %d \n",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port),strlen(msg)); // network_to_host_short for port

	// function for manipulating message before sending the message to the exterior world
	val = ProcessMessageFromInterior(msg,n,addr);
	if(val == false)
		return;

	// send to exterior knowing the correspondence because of association (assoc)
	dprintf("Trying to SEND message to exterior ...");
	n = sendto(listenerSock, msg, strlen(msg), 0, (struct sockaddr *) &assoc.addr, sizeof(assoc.addr));
	if(n < 0)
	{
		error("ReceiveFromInterior - Problem with sending to socket");
	}

	printf("[SEN *->EXT]: from %s:UDP%u : %d \n",
		inet_ntoa(assoc.addr.sin_addr),
		ntohs(assoc.addr.sin_port),strlen(msg)); // network_to_host_short for port

}

int main(int argc, char** argv) {

	init(argc,argv);
	initialize();

	/*Adding the new socket File Descriptor to reading descriptors vector and to readFDS set
	 *From now on, as soon as any client tries to connect to our server, the select function will
	 *trigger this socket.*/
	FD_SET(listenerSock, &readFDS);
	nfds = listenerSock;

	//Used for backing up the FD_SETs as they will be modified by the select function
	fd_set readFDS_Temp;
	int crtSock;

	while (1) {
		//Backing up the FD_SET as it will be modified by the select function
		readFDS_Temp = readFDS;

		/*Calling the select function
		 *Those listed in readfds will be watched to see if characters become available for reading (more precisely, to see  if
		 *a  read  will  not  block and those in writefds will be watched to see if a write will not block
		 */
		if (select(nfds + 1, &readFDS_Temp, NULL, NULL, NULL) == -1)
			error("ERROR in select");

		// CLIENT ---> on listenerSock
		if(FD_ISSET(listenerSock,&readFDS_Temp))
		{
			ReceiveFromExterior();
		}

		/*Now we cycle through all the other file descriptors in readFDS_Temp, and
		 *if one is FD_ISSET(), it means that I can read there. So we do the deed! :P */
		list<ASSOC>::iterator it;
		
		for(it=assocList.begin(); it!=assocList.end(); it++)
		{
			crtSock=it->sock;

			// nothing received through this socket
			if(!FD_ISSET(crtSock,&readFDS_Temp))
				continue;

			// There is something to read from socket
			ReceiveFromInterior(*it);
		}

	}
	close(listenerSock);
	return 0;
}
