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

#include "sdpserver.h"
#include "getipv6.h"
#include "evse_server.h"
#include "sockets.h"
#include "v2gtp.h"
#include "server_setup.h"
#include <stdio.h>
#include "J1772.h"

int InitSDPServer(void)
{
	int sdListen;
	char *port = "15118";

	    /* create a socket descriptor to listen on */
	    sdListen = InitializeServer(port, true, UDP);

	    if (-1 == sdListen)
	    {
	        fprintf(stderr, "Unable to Listen on Port %s\n", port);
	        return EXIT_FAILURE;
	    }

	return sdListen;
}

int ParseSDPReq(int socket, uint8_t* inStream, uint32_t inStreamLength)
{
	uint8_t outStream[MAX_STREAM_SIZE];
	uint32_t outStreamLength;

	uint32_t payloadLength;
	uint16_t payloadType=0;
	uint8_t security, transport, i;
	char ipv6str[INET6_ADDRSTRLEN];


	/* check, if we support this v2gtp version */
	if(inStream[0]!=V2GTP_VERSION && inStream[1]!=V2GTP_VERSION_INV)
		return -1;


	/* check, if we support this payload type*/
	payloadType = inStream[2];
	payloadType = (payloadType << 8 | inStream[3]);

	if(payloadType != V2GTP_SDP_REQUEST_TYPE)
		return -1;


	/* determine payload length*/
	payloadLength = inStream[4];
	payloadLength = (payloadLength << 8 | inStream[5]);
	payloadLength = (payloadLength << 16 | inStream[6]);
	payloadLength = (payloadLength << 24 | inStream[7]);

	if((payloadLength+V2GTP_HEADER_LENGTH)!=inStreamLength)
		return -1;
	if(payloadLength!=2)
			return -1;

	/*Valid SDP V2GTP Header, therefore parse message*/
	security = inStream[8];
	transport = inStream[9];
	if ((security == No_Transport_Layer_Security) && (transport == SDP_TCP))
	{
	  printf("\nValid DC Charging SDP Request\n");
	  printf("Security byte = %d\n",security);
	  printf("Transport byte = %d\n",transport);
	}
	// else
	// {
	// 	printf("\nInvalid DC Charging SDP Request\n");
	// 	return -1;
	// }
	else
	{
		if ((security == Secured_w_TLS))// && (transport == SDP_TCP))
		{
			printf("\nPEV Requests TLS Encrypted Communication\n");//EVCC is requesting a TLS connection, will force non-tls
			printf("Security byte = %d\n",security);
			printf("Transport byte = %d\n",transport);
		}
		else {
		printf("\nInvalid DC Charging SDP Request\n");
		return -1;
		}
	}



    /*Serialize SDP Response*/

	if(getipv6((char *)"eth1", (char *)&ipv6str) != 0)
	{
		fprintf(stderr, "Could not read eth1 interface ipv6 address\n");
	}
	else
	{
	   /*fprintf(stdout, "TEST Out of Function: SECC IPv6 Address: %s\n\n", ipv6str);*/

		/* Convert IP from numbers and dots to binary notation and save to SECC ipv6 address to global structure*/
		if(inet_pton(AF_INET6,ipv6str, (void *)&SECCAddr.sin6_addr.s6_addr) <= 0){
			fprintf(stderr, "Bad IPv6 Address ");
		}
	}

	for(i=0;i<=15;i++)
	{
		outStream[8+i] = SECCAddr.sin6_addr.s6_addr[i];
	}
	/*printf("Network Byte Order:");
				for(i=0;i<=15;i++)
				fprintf(stdout, "%x\n", outStream[8+i]);*/

	/*outStream[8] = 0xfe;
	outStream[9] = 0x80;
	outStream[10] = 0x00;
	outStream[11] = 0x00;
	outStream[12] = 0x00;
	outStream[13] = 0x00;
	outStream[14] = 0x00;
    outStream[15] = 0x00;
	outStream[16] = 0x42;
	outStream[17] = 0x21;
    outStream[18] = 0x00;
	outStream[19] = 0xff;
	outStream[20] = 0xfe;
	outStream[21] = 0x00;
	outStream[22] = 0x02;
	outStream[23] = 0x89;*/

	outStream[24] = 0xFF; /*Port 65535*/
	outStream[25] = 0xFF;

	outStream[26] = No_Transport_Layer_Security;
	outStream[27] = SDP_TCP;
	outStreamLength = 20;
	/* write v2gtp header */
	write_v2gtpHeader(outStream, &outStreamLength, V2GTP_SDP_RESPONSE_TYPE);

	/*call function to send response (outStream) to the Client*/
	if(SDPReply(socket, outStream, outStreamLength) == 0)
	{
		printf("\nSDP Response Sent\n");
	}

	return 0;

}

int ReadSDPReq (int socket, uint8_t* inStream, uint16_t* inStreamLength)
{
uint16_t nread;
SDPpeer_addr_len = sizeof(struct sockaddr_storage);

      nread = recvfrom(socket, inStream, 512, 0,
                (struct sockaddr *) &SDPpeer_addr, &SDPpeer_addr_len);  /*UDP SOCKET*/

	  if (nread < 0)
	  {
		  fprintf(stderr, "Error reading from socket\n");
	    return -1;
	  }

	  else if (nread == 0)
    	    /* End-of-file. */
    	    return -1;

	  else /*Data Read*/
          {
            /*printf("\nReceived %ld bytes from %s.\n\n",
                    (long) nread, host);*/
            *inStreamLength = nread;
	        return 0;
 	      }
}

int SDPReply(int socket, uint8_t* outStream, uint32_t outStreamLength)
{
	if(sendto(socket, outStream, outStreamLength, 0, (struct sockaddr *) &SDPpeer_addr, SDPpeer_addr_len) != outStreamLength)
	{
      fprintf(stderr, "Error sending response\n");
      printf("sendto(): %s\n", strerror(errno));
      return -1;
	}
	return 0;

}



