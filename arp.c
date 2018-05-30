/* $Id: arp.c,v 1.10 2004/11/02 16:10:43 shj Exp $ */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if_arp.h>
#include <net/if.h>
#if HAVE_NET_ETHERNET_H
# include <net/ethernet.h>
#endif
#if HAVE_NET_ETHERTYPES_H
# include <net/ethertypes.h>
#endif
#if HAVE_NET_IF_ARP_H
# include <net/if_arp.h>
#endif

#include <netinet/in.h>
#include <netinet/if_ether.h>

#include "pctl.h"
#include "types.h"
#include "utils.h"

int
send_arp(struct interface_info *ifc, u16 type, u16 ptype,
	struct in_addr *src_ip, struct in_addr *dst_ip,
	u8 *src_hw, u8 *dst_hw, u8 *arp_src_hw, u8 *arp_dst_hw)
	{
	struct arphdr *arp;
	u8 *arp_ptr;
	u8 *ab = NULL;
	int arplen;
	int d;
	int res;

	errno = EINVAL;

	if ((d = open_dl(ifc)) < 0)
		return -1;

	arplen = sizeof(struct ether_header) + sizeof(struct arphdr) + 
		(2 * (ETH_ALEN + sizeof(struct in_addr)));

	ab = (u8 *)malloc(arplen);
	if (ab == NULL)
		return -1;

	memset(ab, 0, arplen);
	if (make_ether_header(ab, src_hw, dst_hw, ptype) < 0) {
		free(ab);
		return -1;
	}

	arp = (struct arphdr *)(ab + sizeof(struct ether_header));
	arp->ar_hrd = htons(ARPHRD_ETHER);
	arp->ar_pro = htons(ETHERTYPE_IP);
	arp->ar_hln = ETH_ALEN;
	arp->ar_pln = sizeof(u32);
	arp->ar_op = htons(type);

	arp_ptr = (u8 *)(arp + 1);
	memcpy((char *)arp_ptr, (char *)arp_src_hw, arp->ar_hln);
	arp_ptr += arp->ar_hln;
	memcpy(arp_ptr, &src_ip->s_addr, arp->ar_pln);
	arp_ptr += arp->ar_pln;
	memcpy((char *)arp_ptr, (char *)arp_dst_hw, arp->ar_hln);
	arp_ptr += arp->ar_hln;
	memcpy(arp_ptr, &dst_ip->s_addr, arp->ar_pln);

	res = write_dl(d, ab, arplen, ifc);
	assert(ab);
	free(ab);
	close_dl(d);

	return res;
}
