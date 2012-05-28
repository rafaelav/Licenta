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

#define MSG_SIZE 500
#define SERVER_ADDR "192.168.1.2"
#define CLIENT_ADDR "192.168.1.6"
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void ReceiveMessageFromMachine(int skt, struct sockaddr_in sockAddr, char* info)
{
	char msg[MSG_SIZE];
	int addrLen,n;
	memset(msg,0,MSG_SIZE);

	// listening on socket
	listen(skt,5);
		
	//struct sockaddr_in sockAddrNew;
	addrLen = sizeof(sockAddr);

	// receiving message
	n = recvfrom(skt, msg, MSG_SIZE, 0, (struct sockaddr *) &sockAddr, (socklen_t *) &addrLen);
		
	if(n < 0)
	{
		error("Problem with reading from socket from client");
	}

	// WHAT program, FROM (IP) who, ON WHAT (port), WHAT message 
	printf("[REC 1->*] %s: from %s:UDP%u : %s \n", info,
		inet_ntoa(sockAddr.sin_addr),
		ntohs(sockAddr.sin_port),msg); // network_to_host_short for port

	close(skt);

	//return sockAddr;
}

void ForwardMessageToMachine(int skt, struct sockaddr_in sockAddr, char* info)
{
	char msg[MSG_SIZE];
	int n;
	n = sendto(skt, msg, strlen(msg)+1, 0, (struct sockaddr *) &sockAddr, sizeof(sockAddr));
	if(n < 0)
	{
		error("Problem with sending via socket to game");
	}
	// WHAT program, FROM (IP) who, ON WHAT (port), WHAT message 
	printf("[FWD *->2] %s: from %s:UDP%u : %s \n", info,
		inet_ntoa(sockAddr.sin_addr),
		ntohs(sockAddr.sin_port),msg); // network_to_host_short for port

	close(skt);
}

void ProxyBehaviour(int sktfdIN, int sktfdOUT, int sktfdREC, int sktfdFWD, struct sockaddr_in remServAddr, struct sockaddr_in remCliAddr, struct sockaddr_in remServAddr2, struct sockaddr_in remCliAddr2, char* info)
{
	struct sockaddr_in sockAddrRec, clientSock;

	// *** RECEIVE *** message from machine 1
	printf("[DEBUG] Trying to receive message from machine 1...\n");
	ReceiveMessageFromMachine(sktfdIN,sockAddrRec,info);
	printf("[DEBUG] Message from machine 1 received!\n");
	// *** END RECEIVE ***

	clientSock = sockAddrRec;


	// *** FORWARD *** message to machine 2
	printf("[DEBUG] Trying to forward message from machine 1 to machine 2...\n");
	ForwardMessageToMachine(sktfdOUT,remServAddr,info);
	printf("[DEBUG] Message forwarded to machine 2!\n");
	// *** END FORWARD ***


	// *** RECEIVE ANSWER *** message from machine 2
	printf("[DEBUG] Trying to receive message from machine 2...\n");
	ReceiveMessageFromMachine(sktfdREC,sockAddrRec,info);
	printf("[DEBUG] Message from machine 2 received!\n");
	// *** END RECEIVE ANSWER ***

	// *** FORWARD ANSWER*** message to machine 1
	//remCliAddr2.sin_port = htons(clientPort);
	//int r4 = bind(sktfdFWD, (struct sockaddr *) &clientSock, sizeof(clientSock));
	//if(r4<0) {error("ooops");}
	printf("[DEBUG] Trying to forward message from machine 2 to machine 1...\n");
	ForwardMessageToMachine(sktfdFWD,remCliAddr2,info);
	printf("[DEBUG] Message forwarded to machine 1!\n");
	// *** END FORWARD ANSWER***
}


int main(int argc, char **argv)
{
	// verifying number of args
	if(argc < 5)
	{
		error("Incorrect args");
	}


	// **** VARIABLES ****
	int sktfdIN, sktfdOUT, sktfdREC, sktfdFWD, portFROMM1, portTOM2;//, portFROMM2, portTOM1;//, cliLen;
	int r1,r2,r3,r4;

	// sockets - program, who sends, game (who we're sending to), used to receive answer from server for client
	struct sockaddr_in remM2Addr, remM1Addr;//, remServAddr2, remCliAddr2;
	struct hostent *h;
	// *** END VARIABLES ***

	// geting ports (IN/OUT) received as param
	portFROMM1 = atoi(argv[1]);	// on this one the program is listening from M1
	portTOM2 = atoi(argv[2]);	// on this one the server is sending to M2
	//portFROMM2 = atoi(argv[3]);	// on this one the program is listening from M2
	//portTOM1 = atoi(argv[4]);	// on this one the server is sending to M1

	// create new sockets (IN, OUT, BACK)
	sktfdIN = socket(AF_INET,SOCK_DGRAM,0);	// used to receive message from a machine
	sktfdOUT = socket(AF_INET,SOCK_DGRAM,0);// used to fwd messages to 2nd machine
	//sktfdREC = socket(AF_INET,SOCK_DGRAM,0);// used to receive answer from 2nd machine
	//sktfdFWD = socket(AF_INET,SOCK_DGRAM,0);// used to fdw message from 2nd machine to 1st machine

	// verifying sockets creation
	if(sktfdIN < 0 || sktfdOUT < 0 || sktfdREC < 0 || sktfdFWD < 0)
	{
		error ("Problem when creating new socket");
	}

	// seting zeros everywhere
	bzero((char *) &remM1Addr, sizeof(remM1Addr));
	bzero((char *) &remM2Addr, sizeof(remM2Addr));
	//bzero((char *) &remCliAddr2, sizeof(remCliAddr2));
	//bzero((char *) &remServAddr2, sizeof(remServAddr));

	// *** M1 *** this is the address used to communicate with M1 (->)
	//h = gethostbyname(CLIENT_ADDR);
	if(h==NULL)
	{
		error ("Problem with IP of destination program (game)");
	}
	remM1Addr.sin_family = AF_INET;		// address family
	remM1Addr.sin_port = htons(portFROMM1);	// port number
	remM1Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//memcpy((char *) &remCliAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);	
	// *** END M1 ***

	// *** M2 *** this is the address used to communicate with M2 (->)
	h = gethostbyname(SERVER_ADDR);		// host of destination program (local host -> game is on same machine)
	if(h==NULL)
	{
		error ("Problem with IP of destination program (game)");
	}
	remM2Addr.sin_family = AF_INET;		// address family
	remM2Addr.sin_port = htons(portTOM2);	// port number
	memcpy((char *) &remM2Addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	// *** END M2 ***


	// *** M2 *** this is the address used to communicate with M2 (<-)
	h = gethostbyname(SERVER_ADDR);		// host of destination program (local host -> game is on same machine)
	if(h==NULL)
	{
		error ("Problem with IP of destination program (game)");
	}
	remServAddr2.sin_family = AF_INET;		// address family
	remServAddr2.sin_port = htons(portFROMM2);	// port number
	memcpy((char *) &remServAddr2.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	// *** END M2 ***

	// *** M1 *** this is the address used to communicate with M1 (<-)
	h = gethostbyname(CLIENT_ADDR);
	if(h==NULL)
	{
		error ("Problem with IP of destination program (game)");
	}
	remCliAddr2.sin_family = AF_INET;		// address family
	remCliAddr2.sin_port = htons(portTOM1);	// port number
	remCliAddr2.sin_addr.s_addr = INADDR_ANY;	
	//memcpy((char *) &remCliAddr2.sin_addr.s_addr, h->h_addr_list[0], h->h_length);	
	// *** END M1 ***

	// tying socket to address
	r1 = bind(sktfdOUT, (struct sockaddr *) &remM2Addr, sizeof(remM2Addr));
	r2 = bind(sktfdREC, (struct sockaddr *) &remServAddr2, sizeof(remM2Addr));
	r3 = bind(sktfdIN, (struct sockaddr *) &remM1Addr, sizeof(remM1Addr));
	r4 = bind(sktfdFWD, (struct sockaddr *) &remCliAddr2, sizeof(remM1Addr));

	// verifying binding
	if(r1 < 0 || r2 < 0 || r3 < 0 || r4 < 0)
	{
		if(r1<0) printf("*->M2 \n");
		if(r2<0) printf("M2->* \n");
		if(r3<0) printf("M1->* \n");
		if(r4<0) printf("*->M1 \n");
		error ("Problem when binding socket to address");
	}


	while(1)
	{

		// calling function which deals with receiving packets and forwarding them to the game
		ProxyBehaviour(sktfdIN, sktfdOUT, sktfdREC, sktfdFWD, remM2Addr, remM1Addr, remServAddr2, remCliAddr2, argv[0]);
	}

	return 0;	
}
