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
int typeOfAttackExterior, typeOfAttackInterior;
int alternate;
int badCount;

void addExtraBytesAttack(char * originalMsg, int length, int &extraInfo, int &count)
{
	char * extension;
	extension =(char *) calloc(NOBYTES, sizeof(char));

	file = fopen(add_bytes,"r");
	// reading from file number (alternate bad/good packets)
	fscanf(file,"%d",&alternate);
	fscanf(file,"%d",&badCount);

	// [alternate] number of messages have been modified -> [alternate] messages should not be modified
	if(count <= alternate)
	{
		//dprintf("count<=alternate/");
		fclose(file);
		free(extension);
		return;
	}
	if(count > alternate+badCount)
	{
		//dprintf("count>2*alternate/");
		count = 0; 	//reset count
		fclose(file);
		free(extension);
		return;
	}

	// reading from file an extension
	fscanf(file, "%s", extension);
	dprintf("extension from file: %s",extension);

	// concatenate to the message the extra bytes
	strcat(originalMsg,extension);
	dprintf("modified message: %s",originalMsg);

	extraInfo = strlen(extension);

	free(extension);
	fclose(file);
}

void changeBytesAttack(char * originalMsg, int length, int &extraInfo)
{
	char ch;
	char *content;
	content =(char *) calloc(NOBYTES, sizeof(char));
	int start;
	int i;

	file = fopen(change_bytes,"r");
	// reading from file an starting byte from which the message will be changed
	fscanf(file, "%d", &start);
	fscanf(file, "%c", &ch);
	if(ch!=' ')
		error("Change bytes file not according to regulations (no_a - no_b)");

	// reading from file content with which the original message will be altered from starting byte
	fscanf(file, "%s", content);

	dprintf("position of starting byte and content to overwrite: %d - %s",start,content);

	// change message
	for(i=start; i<start+strlen(content); i++)
		originalMsg[i] = content[i-start];

	dprintf("modified message: %s",originalMsg);

	fclose(file);
}

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countInt)
{
	switch(typeOfAttackInterior)
	{
		// the attack will lead to adding extra bytes of info to the message such that the application will crash
		case ADDING_BYTES: addExtraBytesAttack(msg, n, extraInfo, countInt); break;
		// the attack will try to modify some bytes in the message thus leading to some patterns being not followed
		case CHANGING_BYTES: changeBytesAttack(msg, n, extraInfo); break;
	}
	return true;
}

bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countExt)
{
	switch(typeOfAttackExterior)
	{
		// the attack will lead to adding extra bytes of info to the message such that the application will crash
		case ADDING_BYTES: addExtraBytesAttack(msg, n, extraInfo, countExt); break;
		// the attack will try to modify some bytes in the message thus leading to some patterns being not followed
		case CHANGING_BYTES: changeBytesAttack(msg, n, extraInfo); break;
	}
	return true;
}

void init(int argc, char **argv)
{
	dprintf(">>> In init -> attacker");
	if(argc <= 3)
		error("[ATTACKER] Not enough arguments");

	if(strlen(argv[1])==0)
		error("[ATTACKER] No tag for behavior outside->inside");

	if(strcmp(argv[1],"a-addbytes") == 0)
	{
		dprintf("Attack -> Adding Bytes");
		typeOfAttackExterior = ADDING_BYTES;

		if(strlen(argv[2])==0)
			error("[ATTACKER] No tag for behavior inside->outside");
		if(strcmp(argv[2],"a-addbytes") == 0)
			typeOfAttackInterior = ADDING_BYTES;
		else
			if(strcmp(argv[2],"a-changebytes") == 0)
				typeOfAttackInterior = CHANGING_BYTES;
			else
				typeOfAttackInterior = UNDEFINED;

		if(strlen(argv[3])==0)
			error("[ATTACKER] No file named added");
		// allocating space for file name
		add_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(add_bytes,argv[3]);
		dprintf("add_bytes: %s",add_bytes);
		return;
	}


	if(strcmp(argv[1],"a-changebytes") == 0)
	{
		dprintf("Attack -> Changing Bytes");
		typeOfAttackExterior = CHANGING_BYTES;

		if(strlen(argv[2])==0)
			error("[ATTACKER] No tag for behavior inside->outside");
		if(strcmp(argv[2],"a-addbytes") == 0)
			typeOfAttackInterior = ADDING_BYTES;
		else
			if(strcmp(argv[2],"a-changebytes") == 0)
				typeOfAttackInterior = CHANGING_BYTES;
			else
				typeOfAttackInterior = UNDEFINED;

		if(strlen(argv[3])==0)
				error("[ATTACKER] No file named added");
		// allocating space for file name
		change_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(change_bytes,argv[3]);
		dprintf("change_bytes: %s",change_bytes);
		return;
	}

	typeOfAttackExterior = UNDEFINED;

	if(strlen(argv[2])==0)
		error("[ATTACKER] No tag for behavior inside->outside");
	if(strcmp(argv[2],"a-addbytes") == 0)
	{
		typeOfAttackInterior = ADDING_BYTES;

		if(strlen(argv[3])==0)
			error("[ATTACKER] No file named added");
		// allocating space for file name
		add_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(add_bytes,argv[3]);
		dprintf("add_bytes: %s",add_bytes);
	}
	else
		if(strcmp(argv[2],"a-changebytes") == 0)
		{
			typeOfAttackInterior = CHANGING_BYTES;

			if(strlen(argv[3])==0)
					error("[ATTACKER] No file named added");
			// allocating space for file name
			change_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
			// keeping the filename from which we will read the bytes to extra add to message
			strcpy(change_bytes,argv[3]);
			dprintf("change_bytes: %s",change_bytes);
		}
		else
			typeOfAttackInterior = UNDEFINED;

}
