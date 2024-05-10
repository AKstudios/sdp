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
#ifndef __SDPCLIENT_H
#define __SDPCLIENT_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

extern int SDPServer;
char *SDPServerName;
struct sockaddr *SDPaiAddr;
struct sockaddr_in6 SECCAddr;

int SDPRequest(void);
int Parse_SDPResponse(uint8_t* inStream, uint32_t inStreamLength, uint32_t* payloadLength);

enum
{
	Secured_w_TLS=0x00,
	No_Transport_Layer_Security=0x10
};

enum
{
	SDP_TCP=0x00,
	SDP_UDP=0x10
};


#endif

