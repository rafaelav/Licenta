/*
 * proxy.h
 *
 *  Created on: May 29, 2012
 *      Author: rafaela
 */

#ifndef PROXY_H_
#define PROXY_H_

#define SERVER_ADDR "192.168.1.2"
#define SERVER_PORT 1500

#define LISTEN_PORT 30002

#define MSG_SIZE 200

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr);
bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr);
void init(int argc, char** argv);

#endif /* PROXY_H_ */
