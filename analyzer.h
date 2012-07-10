/*
 * analyzer.h
 *
 *  Created on: Jun 1, 2012
 *      Author: rafaela
 */

#ifndef ANALYZER_H_
#define ANALYZER_H_

#define NOBYTES 10

#define CHANGE_DIRECTION 1
#define ADDING_BYTES 2
#define CHANGING_BYTES 3
#define UNDEFINED 4
#define MAX_EXT 100
#include "proxy.h"
#include <string.h>
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

#endif /* ANALYZER_H_ */
