/*
 * attacker.cpp
 *
 *  Created on: Jun 1, 2012
 *      Author: rafaela
 */

#include "attacker.h"

char *add_bytes;
char *change_bytes;
FILE *file;
int typeOfAttack;

void changeDirectionAttack(char * originalMsg, int length)
{
	//TODO
}

void addExtraBytesAttack(char * originalMsg, int length)
{
	//TODO
}

void changeBytesAttack(char * originalMsg, int length)
{
	char * extension;
	extension =(char *) calloc(NOBYTES, sizeof(char));

	// reading from file an extension
	file = fopen(add_bytes,"r");
	fscanf(file, "%s", extension);

	// concatenate to the message the extra bytes
	strcat(originalMsg,extension);

	fclose(file);
}

/*void AttackerBehaviour(char * originalMsg, int length, int typeOfAttack)
{
	switch(typeOfAttack)
	{
		// the attack will change bytes from message such as if the player is moving left it will look like it moves right
		case CHANGE_DIRECTION: changeDirectionAttack(originalMsg, length); break;
		// the attack will lead to adding extra bytes of info to the message such that the application will crash
		case ADDING_BYTES: addExtraBytesAttack(originalMsg, length); break;
		// the attack will try to modify some bytes in the message thus leading to some patterns being not followed
		case CHANGING_BYTES: changeBytesAttack(originalMsg, length); break;
	}
}*/

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr)
{
	/*
	 * se poate genera un nr random pentru alegerea atacului
	 */
	//AttackerBehaviour(msg, ADDING_BYTES);
	return true;
}

bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr)
{
	switch(typeOfAttack)
	{
		// the attack will change bytes from message such as if the player is moving left it will look like it moves right
		case CHANGE_DIRECTION: changeDirectionAttack(msg, n); break;
		// the attack will lead to adding extra bytes of info to the message such that the application will crash
		case ADDING_BYTES: addExtraBytesAttack(msg, n); break;
		// the attack will try to modify some bytes in the message thus leading to some patterns being not followed
		case CHANGING_BYTES: changeBytesAttack(msg, n); break;
	}
	return true;
}

void init(int argc, char **argv)
{
	if(argc <= 1)
		return;

	if(strcmp(argv[0],"a-direction") == 0)
	{
		dprintf("Attack -> Change Direction");
		typeOfAttack = CHANGE_DIRECTION;
		return;
	}

	if(strcmp(argv[0],"a-addbytes") == 0)
	{
		dprintf("Attack -> Adding Bytes");
		typeOfAttack = ADDING_BYTES;
		// allocating space for file name
		add_bytes = (char *) calloc(strlen(argv[1])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(add_bytes,argv[1]);
		return;
	}

	if(strcmp(argv[0],"a-changebytes") == 0)
	{
		dprintf("Attack -> Changing Bytes");
		typeOfAttack = CHANGING_BYTES;
		// allocating space for file name
		change_bytes = (char *) calloc(strlen(argv[1])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(change_bytes,argv[1]);
		return;
	}
}