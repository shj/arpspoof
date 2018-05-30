/* $Id: utils.c,v 1.6 2004/11/02 16:03:58 shj Exp $ */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "utils.h"

/*  swiped from ping.c */
u_short
in_cksum(u_short *addr, int len)
{
	int nleft, sum;
	u_short *w;
	union {
		u_short us;
		u_char  uc[2];
	} last;
	u_short answer;

	nleft = len;
	sum = 0;
	w = addr;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		last.uc[0] = *(u_char *)w;
		last.uc[1] = 0;
		sum += last.us;
	}
	
	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	answer = ~sum;                          /* truncate to 16 bits */
	return(answer);
}
	
int
islocalhost(struct in_addr *addr)
{
	
if ((addr->s_addr & htonl(0xff000000)) == htonl(0x7F000000))
	return 1;
if (!(addr->s_addr))
	return 1;
	
return 0;
}

int
getlocaladdr(struct in_addr *src)
{
	char hostname[MAXHOSTNAMELEN + 1];
	
	if (gethostname(hostname, MAXHOSTNAMELEN) < 0)
		return -1;
	if (resolve(hostname, src) < 0)
		return -1;
	
	return 0;
}

int
resolve(const char *host, struct in_addr *addr)
{
	struct hostent *hoste;
	    
	if (inet_aton(host, addr))
		return 0;
	if ((hoste = gethostbyname(host))) {
		memcpy(addr, hoste->h_addr_list[0], sizeof(struct in_addr));
		return 1;
	}
	return -1;
}

char *
inet_itoh(struct in_addr addr)
{
	struct hostent *hptr;
	
	hptr = gethostbyaddr((char *)&addr, sizeof(struct in_addr), AF_INET);
	if (hptr == NULL)
		return inet_ntoa(addr);
	else
		return (hptr->h_name);
	}
	
#ifndef HAVE_BZERO
# define bzero(a,s) memset((a), 0, (s))
#endif
	
#ifndef HAVE_INET_NTOA
char *
inet_ntoa(struct in_addr in)
{
	static char ret[18];
	
	inet_ntop(AF_INET, &in, ret, sizeof ret);
	return ret;
}
#endif
