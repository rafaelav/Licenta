/*
 * attacker.cpp
 *
 *  Created on: Jun 1, 2012
 *      Author: rafaela
 */

#include "defender.h"

void changeDirecectionAttack(char * originalMsg)
{
	//TODO
}

void addExtraBytesAttack(char * originalMsg)
{
	//TODO
}

void changeBytesAttack(char * originalMsg)
{

}

void DefenderBehaviour(char * originalMsg, int typeOfAttack)
{
	switch(typeOfDefence)
	{
		// the attack will change bytes from message such as if the player is moving left it will look like it moves right
		case CHANGE_DIRECTION: changeDirectionDefence(originalMsg); break;
		// the attack will lead to adding extra bytes of info to the message such that the application will crash
		case ADDING_BYTES: addExtraBytesDefence(originalMsg); break;
		// the attack will try to modify some bytes in the message thus leading to some patterns being not followed
		case CHANGING_BYTES: changeBytesDefence(originalMsg); break;
	}
}

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr)
{
	return true;
}

bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr)
{
	return true;
}
