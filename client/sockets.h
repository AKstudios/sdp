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
 * @author Jason D. Harper, Michael Dipperstein
 * @@version 0.7
 * @contact jharper@anl.gov
 *
 ********************************************************************/

#ifndef __SOCKETS_H
#define __SOCKETS_H

#include "sockets.h"
#include "sdpclient.h"
#include "EVCCStateMachine.h"
#include "setipv6.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* for berkeley sockets */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h> /* for inet_ntop */

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#define MAX_BYTE_SIZE 128
#define MAX_STRING_SIZE 128
#define MAX_STREAM_SIZE 100



enum
{
	TCP=0,
	UDP=1
};

int ConnectToServer(const char *serverAddress, const char *port,
    char **serverName, unsigned char ipv6, unsigned char protocol);
int ConnectToSECC (void);
int InterfaceIndex(char *iface_name);
int8_t PollSockets(void);
void closetcp (void);

fd_set active_fd_set;
extern int SECCServer;
char *serverName;



#endif
