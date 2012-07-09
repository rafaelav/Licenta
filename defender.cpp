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
int typeOfDefenseExterior;
int typeOfDefenseInterior;

// returns true when message if NOT ok; returns false when message is ok
bool addExtraBytesDefense(char * originalMsg, int lmsg, int &extraInfo)
{
	int len;
	int i,j;
	bool ruleFits;

	char* ruleType;
	ruleType = (char *) calloc(RULE_LEN,sizeof(char));

	char* ruleLine;
	ruleLine = (char *) calloc(RULE_LEN,sizeof(char));

	char* pattern;
	pattern = (char *) calloc(RULE_LEN,sizeof(char));

	file = fopen(add_bytes,"r");

	while (!feof(file))
	{
		memset(ruleType, 0, RULE_LEN);
		fscanf(file, "%s", ruleType);

		if(strcmp(ruleType,"virus_signature_start") == 0)
		{
			dprintf("Entered pattern - signature");
			memset(ruleLine, 0, RULE_LEN);
			fscanf(file, "%s", ruleLine);

			while(strcmp(ruleLine,"virus_signature_end") != 0)
			{
				if(strcmp(ruleLine,"last:") == 0)
				{
					memset(pattern, 0, RULE_LEN);
					fscanf(file, "%d", &len);
					fscanf(file, "%s", pattern);

					dprintf("Len + Pattern + Word + len_word: %d %s %s %d",len,pattern,originalMsg,lmsg);

					ruleFits = true;
					j=len-1;
					// check if pattern is indeed found at the end of the message
					for(i=lmsg-2; i>=lmsg-len-1; i--)
					{
						// virus signature does not appear in message
						if(pattern[j] != originalMsg[i])
						{
							ruleFits = false;
							dprintf("Rule doesn't fit %c %c",pattern[j],originalMsg[i]);
							break;
						}

						j--;
					}

					if(ruleFits == true)
					{
						dprintf("Rule doesn't fits: %s", originalMsg);
						// erase pattern from the end
						for(i=lmsg-2; i>=lmsg-len-1; i--)
						{
							originalMsg[i]=0;
						}
						dprintf("MODIFIED MESSAGE: %s",originalMsg);

						// update extraInfo to know that proxy should send less bytes then received
						extraInfo = 0-len;
					}

					free(ruleType);
					free(ruleLine);
					free(pattern);
					fclose(file);
					return false;
				}
			}
		}
		else
		/*	if(strcmp(ruleType,"pattern_start") == 0)
			{

			}
			else*/
				error("[DEFENDER] Incorrect rule type in given file");
	}

	free(ruleType);
	free(ruleLine);
	free(pattern);
	fclose(file);
	return false;	// packets will always be fixed so that the game can continue
}

bool changeBytesDefense(char * originalMsg, int lmsg)
{
	/* Example in file:
	 * start_pattern-> new pattern
	 * 2 pattern1	-> from position 2 in message should be pattern1
	 * 8 pattern2	-> from position 8 in message should be pattern2
	 * ...
	 * end_pattern	-> end of pattern
	 * start_pattern
	 * ...
	 * end_pattern
	 */
	bool ruleFits;
	// patternx
	char *line;
	// rule with all patterns between start and end block from config file
	char *rule;
	line = (char *) calloc(NOBYTES*3,sizeof(char));
	rule = (char *) calloc(RULE_LEN,sizeof(char));
	// position from where pattern should start in original_message
	int pos;
	int i;

	// reading from file the rules
	file = fopen(change_bytes,"r");

	while (!feof(file))
	{
		memset(rule, 0, RULE_LEN);
		memset(rule, '*', RULE_LEN-2);
		memset(line, 0, NOBYTES*3);
		fscanf(file, "%s", line);
		dprintf(">>> Am citit: %s ",line);
		if(strcmp(line,"start_pattern")!=0)
		{
			continue;
			//error("[DEFENDER][CHBYTHES]Incorrect structured file - should have blocks starting with start_pattern...end_pattern");
		}
		while(strcmp(line,"end_pattern")!=0)
		{
			memset(line, 0, NOBYTES*3);
			fscanf(file, "%d", &pos);
			fscanf(file, "%s", line);

			// default rule
			if(pos==0 && strcmp(line,"null")==0)
			{
				fclose(file);
				return false;	// message is ok
			}

			for(i=pos; i<pos+strlen(line); i++)
				rule[i]=line[i-pos];

			//memset(line, 0, NOBYTES*3);
		}

		// we presume the rule will fit the message
		ruleFits = true;
		for(i=0; i<strlen(rule); i++)
		{
			if(rule[i] != '*' && lmsg>i && rule[i] != originalMsg[i])
			{
				ruleFits = false;
				break;
			}
		}

		// if a rule was found and it fits the message
		if(ruleFits == true)
		{
			fclose(file);
			free(rule);
			free(line);
			return false;
		}
	}
	free(rule);
	free(line);
	fclose(file);
	return true;	// message did not fit any rule and it was not a default rule anywhere
}

bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countInt)
{
	bool broken = false;
	switch(typeOfDefenseInterior)
	{
		// the defense will prevent packets with extra bytes added to be forwarded to the machine
		case ADDING_BYTES: broken = addExtraBytesDefense(msg, n, extraInfo); break;
		// the defense will prevent messages which don't respect some specified patterns to reach the destination
		case CHANGING_BYTES: broken = changeBytesDefense(msg, n); break;
	}
	if(broken==true)
		return false;	// message will not be forwarded by proxy
	return true;
}

bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countExt)
{
	bool broken = false;
	switch(typeOfDefenseExterior)
	{
		// the defense will prevent packets with extra bytes added to be forwarded to the machine
		case ADDING_BYTES: broken = addExtraBytesDefense(msg, n, extraInfo); break;
		// the defense will prevent messages which don't respect some specified patterns to reach the destination
		case CHANGING_BYTES: broken = changeBytesDefense(msg, n); break;
	}
	if(broken==true)
		return false;	// message will not be forwarded by proxy
	return true;
}

void init(int argc, char **argv)
{
	dprintf(">>> In init -> defender");
	if(argc <= 3)
		error("[DEFENDER] Not enough arguments");

	//dprintf(">>> In init -> attacker -> enough arguments");

	//dprintf(">>> %s",argv[0]);

	if(strlen(argv[1])==0)
		error("[DEFENDER] No tag for behavior outside->inside");


	if(strcmp(argv[1],"d-addbytes") == 0)
	{
		dprintf("Defense -> Adding Bytes");
		typeOfDefenseExterior = ADDING_BYTES;

		if(strlen(argv[2])==0)
			error("[DEFENDER] No tag for behavior inside->outside");
		if(strcmp(argv[2],"d-addbytes") == 0)
			typeOfDefenseInterior = ADDING_BYTES;
		else
			if(strcmp(argv[2],"d-changebytes") == 0)
				typeOfDefenseInterior = CHANGING_BYTES;
			else
				typeOfDefenseInterior = UNDEFINED;

		if(strlen(argv[3])==0)
			error("[DEFENDER] No file named added");
		// allocating space for file name
		add_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(add_bytes,argv[3]);
		dprintf("add_bytes: %s",add_bytes);
		return;
	}


	if(strcmp(argv[1],"d-changebytes") == 0)
	{
		dprintf("Defense -> Changing Bytes");
		typeOfDefenseExterior = CHANGING_BYTES;

		if(strlen(argv[2])==0)
			error("[DEFENDER] No tag for behavior inside->outside");
		if(strcmp(argv[2],"d-addbytes") == 0)
			typeOfDefenseInterior = ADDING_BYTES;
		else
			if(strcmp(argv[2],"d-changebytes") == 0)
				typeOfDefenseInterior = CHANGING_BYTES;
			else
				typeOfDefenseInterior = UNDEFINED;

		if(strlen(argv[3])==0)
				error("[DEFENDER] No file named added");
		// allocating space for file name
		change_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(change_bytes,argv[3]);
		dprintf("change_bytes: %s",change_bytes);
		return;
	}

	typeOfDefenseExterior = UNDEFINED;

	if(strlen(argv[2])==0)
		error("[DEFENDER] No tag for behavior inside->outside");
	if(strcmp(argv[2],"d-addbytes") == 0)
	{
		typeOfDefenseInterior = ADDING_BYTES;

		if(strlen(argv[3])==0)
			error("[DEFENDER] No file named added");
		// allocating space for file name
		add_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		// keeping the filename from which we will read the bytes to extra add to message
		strcpy(add_bytes,argv[3]);
		dprintf("add_bytes: %s",add_bytes);
	}
	else
		if(strcmp(argv[2],"d-changebytes") == 0)
		{
			typeOfDefenseInterior = CHANGING_BYTES;

			if(strlen(argv[3])==0)
					error("[DEFENDER] No file named added");
			// allocating space for file name
			change_bytes = (char *) calloc(strlen(argv[3])+1,sizeof(char));
			// keeping the filename from which we will read the bytes to extra add to message
			strcpy(change_bytes,argv[3]);
			dprintf("change_bytes: %s",change_bytes);
		}
		else
			typeOfDefenseInterior = UNDEFINED;

}
