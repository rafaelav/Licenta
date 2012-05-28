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
} CLIENT;

list<CLIENT> clientList;
fd_set readFDS;

struct sockaddr_in listenerAddr;
int listenerSock;

//Maximum file descriptor of a socket(used for select);
int nfds;

void error(const char *msg) {
	perror(msg);
	exit(1);
}

/*Initializes the server and makes all the necessary settings.*/
void initialize() {
	dprintf("Initializing server...");

	//Opening a new socket
	listenerSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (listenerSock < 0)
		error("ERROR opening socket");

	// seting zeros everywhere
	bzero((char *) &listenerAddr, sizeof(listenerAddr));

	// this is the address used to communicate with incoming connexions
	listenerAddr.sin_family = AF_INET; // address family
	listenerAddr.sin_port = htons(LISTEN_PORT); // port number
	listenerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Binding address to 'srvSock' socket*/
	if (bind(listenerSock, (struct sockaddr*) &listenerAddr,
			sizeof(struct sockaddr)) < 0)
		error("ERROR on binding");

	//Empty readFDS and writeFDS
	FD_ZERO(&readFDS);

	dprintf("Server initialized...");

}

void ReceiveFromExterior()
{
	dprintf("Trying to receive message from exterior...");

	// receivefrom()

	// verifying if socket corresponding to this clint on server side exists

	// if NOT -> create socket + corespondance (CLIENT) + update FD_SET

	// forward message on socket to server world :)
}

void ReceiveFromInterior(CLIENT client)
{
	dprintf("Trying to receive message from interior...");

	// receivefrom()

	// find correspondence -> client address

	// send to client in exterior
}

int main(int argc, char** argv) {

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
		list<CLIENT>::iterator it;
		
		for(it=clientList.begin(); it!=clientList.end(); it++)
		{
			crtSock=it->sock;

			/*Nothing new from this socket;*/
			if(!FD_ISSET(crtSock,&readFDS_Temp))
				continue;

			// There is something to read from socket
			ReceiveFromInterior(*it);
		}

	}
	close(listenerSock);
	return 0;
}

