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
 * @author Jason D. Harper, Michael Dipperstein, Akram S. Ali
 * @@version 0.8
 * @contact jharper@anl.gov, akram.ali@anl.gov
 *
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

/* for berkeley sockets */
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h> /* for inet_ntop */

#ifndef __SOCKETS_H
#define __SOCKETS_H


#ifndef NI_MAXHOST
# define NI_MAXHOST      1025
#endif
#ifndef NI_MAXSERV
# define NI_MAXSERV      32
#endif

#define MAX_BYTE_SIZE 64
#define MAX_STRING_SIZE 64
#define MAX_STREAM_SIZE 512

int PollServer(void);

int sdServer;
int SECCfd;
int sdListen;

char *serverName;
fd_set active_fd_set;
char host[NI_MAXHOST], service[NI_MAXSERV];

volatile unsigned int variable;

enum
{
	TCP=0,
	UDP=1
};

#endif
