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

#include "getipv6.h"
#include <string.h>


int getipv6(char *iface_name, char *ipv6add)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s, i;


	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	/* Walk through linked list, maintaining head pointer so we
	   can free list later */

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;
		if ((ifa->ifa_addr->sa_family != AF_INET6) && (strcmp(ifa->ifa_name, iface_name) != 0))
			continue;

		family = ifa->ifa_addr->sa_family;

		/* For an AF_INET* interface address, display the address */

		if (((family == AF_INET) || (family == AF_INET6)) && (strcmp(ifa->ifa_name, iface_name) == 0))
		{
			s = getnameinfo(ifa->ifa_addr,
					(family == AF_INET) ? sizeof(struct sockaddr_in) :
										  sizeof(struct sockaddr_in6),
										  ipv6add, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				return -1;
			}

			for(i=0; i<=strlen(ipv6add); i++)
			{
				if(ipv6add[i] == '%' )
				{
				   break;
				}
			}
				ipv6add[i] = 0; /*remove % and interface index from ipv6 address*/
			    /*printf("\nSECC IPv6 address: %s\n", ipv6add);*/
		}
	}

	freeifaddrs(ifaddr);

	return 0;

}
