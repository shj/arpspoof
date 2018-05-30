/* $Id: pctl.c,v 1.7 2004/11/02 16:01:35 shj Exp $ */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if_arp.h>
#include <net/if.h>
#if HAVE_NET_ETHERNET_H
# include <net/ethernet.h>
#endif
#if HAVE_NET_ETHERTYPES_H
# include <net/ethertypes.h>
#endif

#include <netinet/in.h>
#if HAVE_NETINET_IF_ETHER_H
# include <netinet/if_ether.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "pctl.h"
#include "types.h"
#include "utils.h"

extern char zero[];

int
make_ether_header(void *p, u8 *src_hw, u8 *dst_hw, u16 type)
{
	struct ether_header *ether;
  
	ether = (struct ether_header *)p;
	memcpy((char *)ether->ether_dhost, (char *)dst_hw, ETH_ALEN);
	memcpy((char *)ether->ether_shost, (char *)src_hw, ETH_ALEN);
	ether->ether_type = htons(type);

	return 0;
}
