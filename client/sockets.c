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

#include "sockets.h"
#include "apptimer.h"
#include "EVCCStateMachine.h"

// A place to throw away values
volatile int bitbucket;

char *SockAddrToText(struct sockaddr *address);
int SECCServer;

int ConnectToServer(const char *serverAddress, const char *port,
    char **serverName, unsigned char ipv6, unsigned char protocol)
{
    struct addrinfo infoHint;
    struct addrinfo *infoResult, *rp;
    struct sockaddr *aiAddr;
    int result, x;
    int s = 0;

     /* connect socket to server */
    memset(&infoHint, 0, sizeof(infoHint));

    if (ipv6)
    {
        infoHint.ai_family = AF_INET6;      /* IPv6 */
    }
    else
    {
        infoHint.ai_family = AF_INET;       /* IPv4 */
    }

    switch (protocol)
   	{
   	case TCP:
   		infoHint.ai_protocol = IPPROTO_TCP;     /* TCP/IP */
   		infoHint.ai_socktype = SOCK_STREAM;     /* streams */
   		break;
   	case UDP:
   		infoHint.ai_protocol = IPPROTO_UDP; /*User Datagram Protocol*/
   		infoHint.ai_socktype = SOCK_DGRAM;
   		break;
   	default:
   		infoHint.ai_protocol = IPPROTO_TCP; /* TCP/IP */
   		infoHint.ai_socktype = SOCK_STREAM;     /* streams */
   		break;
   	}




    infoResult = NULL;

    result = getaddrinfo(serverAddress, port, &infoHint, &infoResult);



    if (0 != result)
    {
        perror("Error Getting Address Info");
        freeaddrinfo(infoResult);
        return -1;
    }

    /***********************************************************************
    * attempt to connect using the first address returned by the call to
    * getaddrinfo.  Maybe I should try others if first address fails.
    ***********************************************************************/

    /* create socket for connecting to server */
    for (rp = infoResult; rp != NULL; rp = rp->ai_next) {
      s = socket(infoResult->ai_family, infoResult->ai_socktype, infoResult->ai_protocol);

      if (s == -1)
    	 continue;
      if (protocol == TCP)
      x = connect(s, rp->ai_addr, rp->ai_addrlen);
      /*printf("Connect(): %s\n", strerror(errno));*/

      if (x != -1)
        break; /*Success*/

      if ((s != -1) && (protocol == UDP))
    	  break; /*Successfully created UDP socket*/

      close(s);
    }
    if (rp == NULL) {               /* No address succeeded */
      perror("Error Connecting to the Server");
      close(s);
      freeaddrinfo(infoResult);
      return -1;
    }

    /* save connection information */
    aiAddr = infoResult->ai_addr;

    if(protocol == UDP)
    {
      SDPaiAddr = aiAddr;
    }
    *serverName = SockAddrToText((struct sockaddr *)aiAddr);
    freeaddrinfo(infoResult);

    return s;
}

char *SockAddrToText(struct sockaddr *address)
{
    char addrBuffer[INET6_ADDRSTRLEN];
    char *asText;
    unsigned short port;

    addrBuffer[0] = '\0';

    /* use standard non-windows function to get address and port */
    if (address->sa_family == AF_INET)
    {
        inet_ntop(AF_INET, &(((struct sockaddr_in *)address)->sin_addr),
            addrBuffer, INET_ADDRSTRLEN);

        port = ((struct sockaddr_in *)address)->sin_port;
    }
    else if (address->sa_family == AF_INET6)
    {
        inet_ntop(AF_INET6,
            &(((struct sockaddr_in6 *)address)->sin6_addr),
            addrBuffer, INET6_ADDRSTRLEN);

        port = ((struct sockaddr_in6 *)address)->sin6_port;
    }
    else
    {
        /* unknown protocol */
        return NULL;
    }

    asText = (char *)malloc((strlen(addrBuffer) + 7) * sizeof(char));

    if (asText != NULL)
    {
        port = ntohs(port);
        sprintf(asText, "%s:%d", addrBuffer, port);
    }

    return asText;
}

int ConnectToSECC (void)
{
#ifndef SDP
	const char *serverAddress = "fe80::4221:ff:fe00:289%7"; /*use %4 for iMX28 eth0* or %7 for QCA PL16 eth1*/ /*use %5 for imx28 eth1*/
	const char *serverPort = "2048";/*"65535";*/
#endif
#ifdef SDP
	char ipv6str[INET6_ADDRSTRLEN];
	char Port[6] = {0};
	char IfIndex[4] = {0};
	int x;
	uint8_t ifindex;
	if(inet_ntop(AF_INET6, (void *)&SECCAddr.sin6_addr.s6_addr, ipv6str, INET6_ADDRSTRLEN) == NULL)
	{
		fprintf(stderr, "Could not convert byte to address\n");
		fprintf(stderr, "%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ifindex = InterfaceIndex((char *)"eth1");
	if(ifindex < 0)
	{
		fprintf(stderr, "eth1 interface not found\n");
		return EXIT_FAILURE;
	}
	/*else
	printf("ifindex = %d\n", ifindex);*/

	sprintf(IfIndex, "%d", ifindex); /*convert int to string*/

	/*printf("Stringlength IfIndex: %d\n", strlen(IfIndex));
	printf("String IfIndex: %s\n", IfIndex);*/

   /*Configure SECC Ipv6 Address before connecting to Server by appending what interface to utilize*/
	int len = strlen(ipv6str);
	ipv6str[len] = '%';
	for(x = 0; x<strlen(IfIndex); x++)
	{
	  ipv6str[len+1+x] = IfIndex[x];
	}
	ipv6str[len+1+x] = '\0';
	/*printf("New ip address %s\n", ipv6str);*/
	const char *serverAddress = ipv6str;

	/*Configure SECC Ipv6 Port before connecting to Server*/
	const char *serverPort = (char *)&Port;
	/*printf("Struct Port: %d\n", SECCAddr.sin6_port);*/
	sprintf(Port, "%d", SECCAddr.sin6_port); /*convert int to string*/
	/*printf("String Port: %s\n", Port);*/

#endif

    SECCServer = ConnectToServer(serverAddress, serverPort, &serverName, TRUE, TCP);

    if (-1 == SECCServer)
    {
        fprintf(stderr, "Unable to Connect to %s\n", serverAddress);
        return EXIT_FAILURE;
    }

    printf("Connected to SECC at %s\n", serverName);
    /*free(serverName);*/
    /*close(sdServer);*/
    return 0;
}

int InterfaceIndex(char *iface_name)
{
	int sockfd;
	struct ifreq ifr;
	struct sockaddr_in6 sin;
	//struct in6_ifreq ifr6;

	sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
	if(sockfd == -1){
		fprintf(stderr, "Could not get socket.\n");
		return -1;
	}

	/* get interface name */
	strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);
	memset(&sin, 0, sizeof(struct sockaddr));

	/* Get interface index*/
	  if (ioctl(sockfd, SIOGIFINDEX, &ifr) < 0) {
		fprintf(stderr, "Cannot get interface index. ");
		perror("SIOGIFINDEX");
		return -1;
		}
	 //ifr6.ifr6_ifindex = ifr.ifr_ifindex;

	 return ifr.ifr_ifindex;
}

int8_t PollSockets(void)
{
	fd_set read_fd_set;
	int i;
	struct timeval timeout;
	int16_t nread;
	struct signalfd_siginfo info;

		timeout.tv_sec  = 0;
		timeout.tv_usec = 100; /*wait 100 us for input on sockets*/

		/* Block for timeout until input arrives on one or more active sockets.  */
		  read_fd_set = active_fd_set;
		  if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout) < 0)
			{
			  perror ("select");
			  return -1;
			}

		      /* Service all the sockets with input pending.  */
		   for (i = 0; i < FD_SETSIZE; ++i)
		    {
				if (FD_ISSET (i, &read_fd_set))
				  {
					   if (i == killsfd) /*SIGINT*/
						 {
							 logtime();
							 printf("Closing Application.\n");
							 closetcp();
							 exit(0);
						 }
					   else if (i == timersfd) /*Oscillator Timer*/
						 {
							 if(CurrentState == PRECHARGE_CableCheck) /*Charger exceeded 40 sec Isolation Time*/
							 {
								 logtime();
								 fprintf(stderr, "Error Isolation Monitoring Timeout\n");
								 closetcp();
							 }

							 if (CurrentState == PRECHARGE_PreCharge) /*Charger exceeded 7 sec Precharge Time*/
							 {
								 logtime();
								 fprintf(stderr, "Error PreCharge Timeout\n");
								 closetcp();
							 }
							 if (CurrentState == ENERGY_TRANSFER) /*250 ms has exceeded*/
							 {
								 g_flag_newIvaluerecv = 1; /*set the global flag (send new current demand request)*/
							 }

							 /*read file descriptor*/
							nread = read(timersfd, &info, sizeof (info));
							 if (nread < 0)
							  {
								logtime();
								fprintf(stderr, "Error reading oscillator timer alarm.\n");
							  }
						 }
					   else if (i == alarmsfd) /*Sequence Timer*/
						 {
							 logtime();
							 fprintf(stderr, "Error EVCC Sequence Timeout.\n");
							 /*
							  * 2.  Terminate V2G Session
							  * 3.  Close TCP connection*/
							 /*disable EVSE output*/
							 closetcp();

							 /*read file descriptor*/
							nread = read(alarmsfd, &info, sizeof (info));
							 if (nread < 0)
							  {
								logtime();
								fprintf(stderr, "Error reading sequence timer alarm.\n");
							  }
						 }
				  }
			 }
		     return 0;
}

void closetcp (void)
{
	char command[100];
	// Command(B, Standby, 0); /*Place Ch B into Standby Mode $180*/
	usleep(50000);
	gpio_set_value(PEV_READY_NAME, 0); /*Change Pilot to state B*/

	if (CurrentState > INITILIZATION_ChargeParameterDiscovery)
	{
	  bitbucket = system("/home/spec/scripts/inletunlock.sh");  //UnLock the Inlet
	  printf("\nUnlocking Inlet\n");
	}

	if ((CurrentState >= INITILIZATION_HandShake)&&(CurrentState < SHUTDOWN_Connected))
	{
	logtime();
	fprintf(stderr, "EVCC Closing TCP Connection.\n");
	close(SECCServer); /*close TCP connection*/
	FD_CLR (SECCServer, &active_fd_set); //remove socket from active fd set
	CurrentState = SHUTDOWN_Connected;
	}

#ifdef wireshark
	if(g_flag_wireshark_log)
	{
		sprintf(command, "pkill tcpdump");
		bitbucket =system(command);/*kill the wireshark file*/
		printf("Saved Wireshark Trace File\n");
		g_flag_wireshark_log = 0;
	}
#endif
	 /*reset battery to allow recharge on next plugin*/
	 g_flag_charged = 0;
	 g_flag_bulk = 0;
	 currentDemand.ChargingComplete = 0;
	 sleep(3);

#ifdef test
	 SampleJ1772();
	 if (((j1772.PILOTSTATE == STATE_C2) || (j1772.PILOTSTATE == STATE_D2) || ((j1772.PILOTSTATE == STATE_B2)&&(CurrentState < SHUTDOWN_SessionStop))))/*if Pilot in State D2, C2, or B2 (oscillator on) means EVSE is still good to go*/
	 {
		 CurrentState = DISCONNECTED;; /*this will allow a communication to start without the driver having to unplug/replug*/
	 }
	 else
	 {
		 if(CurrentState == SHUTDOWN_SessionStop)
		 {
		   CurrentState = SHUTDOWN_Connected; /*this will cause the driver to unplug before new session can start*/
		 }
		 else
			 CurrentState = DISCONNECTED;
	 }

	 system("/home/spec/scripts/StateA.sh"); /*shutoff oscillator*/

	 sprintf(command, "ps aux | awk '/tcpdump/ {print \"kill \"$1}' | sh");
	 system(command);/*kill the wireshark file*/

	 printf("Saved Wireshark Trace File\n");

	 EVSEResponseCode = ok; /*reset EVSEresponscode*/
	 alarm(0); 	/*clear the alarm*/
	 StateC_Donly = 0; /*clear flag*/
	 sleep(3);
#endif

}

