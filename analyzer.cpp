/*
 * attacker.cpp
 *
 *  Created on: Jun 1, 2012
 *      Author: rafaela
 */

#include "analyzer.h"

FILE *foutExt,*foutInt;
bool AnalyzeInt;
bool AnalyzeExt;


void exterior(char *msg, int n, struct sockaddr_in addr)
{
	int i;
	dprintf("[ANALYZER] In function ProcessFromExt");
	// limits for message
	fprintf(foutExt, "\n------------------MESSAGE----------------------\n");
	dprintf("[ANALYZER] MESSAGE delim ");
	// print source IP
	fprintf(foutExt,"source IP: %s\n", inet_ntoa(addr.sin_addr));
	dprintf("[ANALYZER] IP");
	// print source port
	fprintf(foutExt,"source PORT: %u\n", ntohs(addr.sin_port));
	dprintf("[ANALYZER] port");
	// print message length
	fprintf(foutExt,"length: %u\n", n);
	dprintf("[ANALYZER] len");
	// print message in hexa
	fprintf(foutExt,"data: ");
	for(i=0; i<n; i++)
		fprintf(foutExt,"%x ", (unsigned char) msg[i]);
	fprintf(foutExt,"\n");
	dprintf("[ANALYZER] for data");
	// limits for message
	fprintf(foutExt, "\n-----------------------------------------------\n");
	dprintf("[ANALYZER] end");
}

void interior(char *msg, int n, struct sockaddr_in addr)
{
	int i;

	// limits for message
	fprintf(foutInt, "\n------------------MESSAGE----------------------\n");

	// print source IP
	fprintf(foutInt,"source IP: %s\n", inet_ntoa(addr.sin_addr));

	// print source port
	fprintf(foutInt,"source PORT: %u\n", ntohs(addr.sin_port));

	// print message length
	fprintf(foutInt,"length: %u\n", n);

	// print message in hexa
	fprintf(foutInt,"data: ");
	for(i=0; i<n; i++)
		fprintf(foutInt,"%x ", (unsigned char) msg[i]);
	fprintf(foutInt,"\n");

	// limits for message
	fprintf(foutInt, "\n-----------------------------------------------\n");
}
bool ProcessMessageFromInterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countInt)
{
	if(AnalyzeInt == true)
		interior(msg,n,addr);
	return true;
}
bool ProcessMessageFromExterior(char *msg, int n, struct sockaddr_in addr, int &extraInfo, int &countExt)
{
	if(AnalyzeExt == true)
		exterior(msg,n,addr);
	return true;
}
void init(int argc, char** argv)
{
	dprintf(">>> In init -> analyzer");
	if(argc <= 4)
		error("[ATTACKER] Not enough arguments");

	AnalyzeExt = false;
	AnalyzeInt = false;

	// testing parameters
	if(strlen(argv[1]) == 0)
	{
		error("[ANALYZER] No tag for behavior outside->inside");
	}

	if(strlen(argv[2]) == 0)
	{
		error("[ANALYZER] No tag for behavior inside->outside");
	}

	if(strlen(argv[3]) == 0)
	{
		error("[ANALYZER] No file for behavior outside->inside");
	}

	if(strlen(argv[4]) == 0)
	{
		error("[ANALYZER] No file for behavior inside->outside");
	}

	if(strcmp(argv[1],"analyze") == 0)
	{
		// output file for exterior case
		//fileExt = (char *) calloc(strlen(argv[3])+1,sizeof(char));
		//strcpy(fileExt,argv[3]);
		foutExt = fopen(argv[3],"w+");
		AnalyzeExt = true;
	}

	if(strcmp(argv[2],"analyze") == 0)
	{
		// output file for interior case
		//fileInt = (char *) calloc(strlen(argv[4])+1,sizeof(char));
		//strcpy(fileInt,argv[4]);
		foutInt = fopen(argv[4],"w+");
		AnalyzeInt = true;
	}
}
