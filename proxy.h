/*
 * proxy.h
 *
 *  Created on: May 29, 2012
 *      Author: rafaela
 */

#ifndef PROXY_H_
#define PROXY_H_

//#define SERVER_ADDR "192.168.1.2"
#define SERVER_PORT 1500

#define LISTEN_PORT 30002

#define MSG_SIZE 500
#define MAX_MSG_INIT 20

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countInt);
bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countExt);
void init(int argc, char** argv);
void error(const char *msg);

#endif /* PROXY_H_ */
