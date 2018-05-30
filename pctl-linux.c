/* $Id: pctl-linux.c,v 1.11 2004/11/02 16:00:49 shj Exp $ */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "pctl.h"

#define MAX_ETH_NUM 12

int
open_dl(struct interface_info *ifc)
{

	return socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL));
}

int
close_dl(int d)
{

	return close(d);
}

int
write_dl(int d, void *msg, int len, struct interface_info *ifc)
{
	struct sockaddr to;
	int tolen = sizeof(struct sockaddr);

	bzero(&to, tolen);
	to.sa_family = PF_PACKET;
	strncpy(to.sa_data, ifc->if_name, sizeof(to.sa_data));
	to.sa_data[sizeof(to.sa_data) - 1] = '\0';

	return sendto(d, (char *)msg, len, 0, (struct sockaddr *)&to, tolen);
}

int
ckifname(char *ifname, struct ifreq *ifr, int sd)
{

	ifr->ifr_name[IFNAMSIZ - 1] = '\0';
	strncpy(ifr->ifr_name, ifname, IFNAMSIZ - 1);

	return ioctl(sd, SIOCGIFFLAGS, ifr);
}
