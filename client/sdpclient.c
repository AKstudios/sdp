/*
 * Copyright (C) 2007-2012 Argonne National Laboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************
 *
 * @author Jason D. Harper
 * @@version 0.7
 * @contact jharper@anl.gov
 *
 ********************************************************************/

#include "sdpclient.h"
#include "sockets.h"
#include "v2gtp.h"

#include <stdio.h>

int SDPServer;

int SDPRequest(void)
{
	char ipv6str[INET6_ADDRSTRLEN] = "FF02::1";
	const char *serverAddress = ipv6str;
	const char *serverPort = "15118";
	char IfIndex[4] = {0};
	int x;
	uint8_t ifindex;

	ifindex = InterfaceIndex((char *)"eth1");
	if(ifindex < 0)
	{
		fprintf(stderr, "eth1 interface not found\n");
		return EXIT_FAILURE;
	}

	sprintf(IfIndex, "%d", ifindex); /*convert int to string*/

	/*Configure SDP Ipv6 Address before connecting to Server by appending what interface to utilize*/
	int len = strlen(ipv6str);
	/*printf("Stringlength address: %d\n", len);*/
	ipv6str[len] = '%';
	for(x = 0; x<strlen(IfIndex); x++)
	{
		ipv6str[len+1+x] = IfIndex[x];
	}
	ipv6str[len+1+x] = '\0';

	printf("Trying to Find SDP Server on %s:%s\n", ipv6str, serverPort);  /*<---- DO NOT REMOVE, for some reason the CR is needed or the SDP client does not find the SDP server?*/

    SDPServer = ConnectToServer(serverAddress, serverPort, &SDPServerName, 1, UDP);  /*create a UDP socket to send SDP request*/

    if (-1 == SDPServer)
    {
        fprintf(stderr, "Unable to Find SDP Server\n");
        return 1;
    }

    printf("\nFound SDP Server on %s\n", SDPServerName);

    return 0;
}

int Parse_SDPResponse(uint8_t* inStream, uint32_t inStreamLength, uint32_t* payloadLength)
{
	uint16_t payloadType=0;
	uint8_t ipv6[16];
	uint16_t SECCPort;
	uint8_t i, security, transport;
	char ipv6str[INET6_ADDRSTRLEN];


	/* check, if we support this v2gtp version */
	if(inStream[0]!=V2GTP_VERSION && inStream[1]!=V2GTP_VERSION_INV)
		return -1;


	/* check, if we support this payload type*/
	payloadType = inStream[2];
	payloadType = (payloadType << 8 | inStream[3]);

	if(payloadType != V2GTP_SDP_RESPONSE_TYPE)
		return -1;


	/* determine payload length*/
	*payloadLength = inStream[4];
	*payloadLength = (*payloadLength << 8 | inStream[5]);
	*payloadLength = (*payloadLength << 16 | inStream[6]);
	*payloadLength = (*payloadLength << 24 | inStream[7]);

	if((*payloadLength+V2GTP_HEADER_LENGTH)!=inStreamLength)
		return -1;
	if(*payloadLength!=20)
			return -1;

	/*Valid V2GTP Header, therefore parse message*/

	security = inStream[26];
	transport = inStream[27];
	if ((security != 0x10)||(transport != 0x00))
	{
		printf("Invalid SDP Security or Transport Protocol\n");
		return -1;
	}

	for(i=0;i<=15;i++)
	{
		ipv6[i] = inStream[8+i];
	}
	SECCPort = inStream[24] << 8;
	SECCPort = SECCPort | inStream[25];

	SECCAddr.sin6_port = SECCPort;  /*save SECC ipv6 port to global structure*/


	if(inet_ntop(AF_INET6, (void *)ipv6, ipv6str, INET6_ADDRSTRLEN) == NULL)
	{
		fprintf(stderr, "Could not convert byte to address\n");
		fprintf(stderr, "%s\n", strerror(errno));
	}

	/* Convert IP from numbers and dots to binary notation and save to SECC ipv6 address to global structure*/
	if(inet_pton(AF_INET6,ipv6str, (void *)&SECCAddr.sin6_addr.s6_addr) <= 0){
		fprintf(stderr, "Bad IPv6 Address ");
	}
	/*printf("Network Byte Order:");
		for(i=0;i<=15;i++)
		fprintf(stdout, "%x", SECCAddr.sin6_addr.s6_addr[i]);
		printf("\n");*/


	return 0;
}



