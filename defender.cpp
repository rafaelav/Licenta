/*
 * defender.cpp
 *
 *  Created on: Jun 1, 2012
 *      Author: rafaela
 */

#include "defender.h"

char *add_bytes;
char *change_bytes;
FILE *file;
int typeOfDefense;

/*void changeDirecectionDefense(char * originalMsg, int lmsg)
{
}*/

// returns true when message if NOT ok; returns false when message is ok
bool addExtraBytesDefense(char * originalMsg, int lmsg)
{
	// Line in file example: start n a1 a2 ... an n_message

	// where pattern starts in message
	int start;
	// length of pattern from message
	int patternLen;
	// pattern which should be in message
	char *pattern;
	// if message has pattern then the length of the message will not exceed maxLen
	int maxLen;
	int i;
	bool ruleFits;

	pattern = (char *) calloc(patternLen+1, sizeof(char));

	// reading from file the rules
	file = fopen(add_bytes,"r");
	while (!feof(file))
	{
		ruleFits = true;
		// reading rules by lines
		fscanf(file, "%d", &start);
		fscanf(file, "%d", &patternLen);
		fscanf(file, "%d", &maxLen);
		fscanf(file, "%s", pattern);

		dprintf("[DEFENDER][ADD_BYTES] Read rule: %d %d %d %s", start, patternLen, maxLen, pattern);

		// default rule
		if(start == 0 && patternLen ==0)
		{
			// checking if message has extra bytes added which are not supposed to be there => rule broken => message not ok
			if(strlen(originalMsg) > maxLen)
				return true;
			return false;
		}

		// checking patter
		for(i=start; i<start+patternLen; i++)
			if(originalMsg[i] != pattern[i-start])
			{
				ruleFits = false;
				break;
			}

		// rule applies to message -> we have maximum length per respective message
		if(ruleFits)
		{
			// checking if message has extra bytes added which are not supposed to be there => rule broken => message not ok
			if(strlen(originalMsg) > maxLen)
				return true;
		}
	}

	// if there is no default rule and all rules mentioned in the file do not apply by default message will be considered correct
	return false;
}

bool changeBytesDefense(char * originalMsg, int lmsg)
{
	return true;
}

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr)
{
	bool broken = false;
	switch(typeOfDefense)
	{
//		case CHANGE_DIRECTION: changeDirectionDefense(msg, n); break;
		// the defense will prevent packets with extra bytes added to be forwarded to the machine
		case ADDING_BYTES: broken = addExtraBytesDefense(msg, n); break;
		// the defense will prevent messages which don't respect some specified patterns to reach the destination
		case CHANGING_BYTES: broken = changeBytesDefense(msg, n); break;
	}
	if(broken)
		return false;	// message will not be forwarded by proxy
	return true;
}

bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr)
{
	bool broken = false;
	switch(typeOfDefense)
	{
//		case CHANGE_DIRECTION: changeDirectionDefense(msg, n); break;
		// the defense will prevent packets with extra bytes added to be forwarded to the machine
		case ADDING_BYTES: addExtraBytesDefense(msg, n); break;
		// the defense will prevent messages which don't respect some specified patterns to reach the destination
		case CHANGING_BYTES: changeBytesDefense(msg, n); break;
	}
	if(broken)
		return false;	// message will not be forwarded by proxy
	return true;
}

void init(int argc, char **argv)
{
	dprintf(">>> In init -> defender");
	if(argc <= 1)
		return;

	dprintf(">>> In init -> defender -> enough arguments");

	dprintf(">>> %s",argv[0]);

	if(strcmp(argv[1],"d-direction") == 0)
	{
		dprintf("Defense -> Change Direction");
		typeOfDefense = CHANGE_DIRECTION;
		return;
	}

	if(strcmp(argv[1],"d-addbytes") == 0)
	{
		dprintf("Defense -> Adding Bytes");
		typeOfDefense = ADDING_BYTES;
		// allocating space for file name
		add_bytes = (char *) calloc(strlen(argv[1])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(add_bytes,argv[2]);
		dprintf("add_bytes: %s",add_bytes);
		return;
	}

	if(strcmp(argv[1],"d-changebytes") == 0)
	{
		dprintf("Defense -> Changing Bytes");
		typeOfDefense = CHANGING_BYTES;
		// allocating space for file name
		change_bytes = (char *) calloc(strlen(argv[1])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(change_bytes,argv[2]);
		dprintf("change_bytes: %s",change_bytes);
		return;
	}
}
