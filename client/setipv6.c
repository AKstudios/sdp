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
 * @@version 0.1
 * @contact jharper@anl.gov
 *
 ********************************************************************/

#include "setipv6.h"

int setipv6(char *iface_name, char *ip_addr)
{
	char ipv6str[INET6_ADDRSTRLEN];
	int s;

	if(!iface_name)
		return -1;

	int sockfd;
	struct ifreq ifr;
	struct sockaddr_in6 sin;
	struct in6_ifreq ifr6;

	sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
	if(sockfd == -1){
		fprintf(stderr, "Could not get socket.\n");
		return -1;
	}

	/* get interface name */
	strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);
	memset(&sin, 0, sizeof(struct sockaddr));

	/* Read interface flags */
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
		fprintf(stderr, "ifdown: shutdown ");
		perror(ifr.ifr_name);
		return -1;
	}

	/*
	* Expected in <net/if.h> according to
	* "UNIX Network Programming".
	*/
	#ifdef ifr_flags
	# define IRFFLAGS       ifr_flags
	#else   /* Present on kFreeBSD */
	# define IRFFLAGS       ifr_flagshigh
	#endif

	/* If interface is down, bring it up*/
	if (!(ifr.IRFFLAGS & IFF_UP)) {
		fprintf(stdout, "Device is currently down..setting up.-- %u\n",ifr.IRFFLAGS);
		ifr.IRFFLAGS |= IFF_UP;
		if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
			fprintf(stderr, "ifup: failed ");
			perror(ifr.ifr_name);
			return -1;
		}
	}

	sin.sin6_family = AF_INET6;
	sin.sin6_port = 0;

	/*printf("Passed IP: %s\n\n", ip_addr);*/

	/* Convert IP from numbers and dots to binary notation*/
	if(inet_pton(sin.sin6_family,ip_addr, (void *)&sin.sin6_addr) <= 0){
		fprintf(stderr, "Bad IPv6 Address ");
	}

	/*Must make a unique IP address different from passed in IP*/
	if (sin.sin6_addr.s6_addr[15] < 0XFF)
		sin.sin6_addr.s6_addr[15] = sin.sin6_addr.s6_addr[15] + 1;
	else
		sin.sin6_addr.s6_addr[15] = 0x00;

	memcpy((char *) &ifr6.ifr6_addr, (char *) &sin.sin6_addr, sizeof(struct in6_addr));


		/* Set interface address*/
	  if (ioctl(sockfd, SIOGIFINDEX, &ifr) < 0) {
		fprintf(stderr, "Cannot get interface index. ");
		perror("SIOGIFINDEX");
		return -1;
		}
	 ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	 /*printf("ifindex = %d", ifr.ifr_ifindex);*/
	 ifr6.ifr6_prefixlen = 64;

	  if (ioctl(sockfd, SIOCSIFADDR, &ifr6) < 0) {
		fprintf(stderr, "Cannot set IP address. ");
		perror("SIOCSIFADDR");
		return -1;
		}

	  if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "Cannot get hpgp interface MAC ");
		perror("SIOCSIFADDR");
		return -1;
		}

	close(sockfd);
	/*inform user of new EVCC Global IP address*/
	if(inet_ntop(AF_INET6, (void *)&sin.sin6_addr, ipv6str, INET6_ADDRSTRLEN) == NULL)
		{
			fprintf(stderr, "Could not convert byte to address\n");
			fprintf(stderr, "%s\n", strerror(errno));
		}
	else
	{
		printf("MAC Address: ");
		for( s = 0; s < 6; s++ )
		{
			printf("%.2X ", (unsigned char)ifr.ifr_hwaddr.sa_data[s]);
		}
    	printf("\n");
		fprintf(stdout, "EVCC New Global IPv6 Address: %s\n\n", ipv6str);
	}

	return 0;
}

int get_mac(char *iface_name, char *mac_addr)
{
	int s;
	int sockfd;
	struct ifreq ifr;

	if(!iface_name)
		return -1;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Could not get socket.\n");
		return -1;
	}

	memset(&ifr, 0x00, sizeof(ifr));

	strcpy(ifr.ifr_name, iface_name);

    ioctl(sockfd, SIOCGIFHWADDR, &ifr);

    close(sockfd);

	/*printf("MAC Address: ");
	for( s = 0; s < 6; s++ )
	{
		printf("%.2X ", (unsigned char)ifr.ifr_hwaddr.sa_data[s]);
	}
	printf("\n"); */

	strcpy(mac_addr, ifr.ifr_hwaddr.sa_data);

	return 0;

}
