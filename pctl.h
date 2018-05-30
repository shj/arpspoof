/* $Id: pctl.h,v 1.5 2004/11/02 16:03:08 shj Exp $ */

#ifndef PCTL_H
#define PCTL_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/socket.h>
#include <sys/types.h>

#if HAVE_NET_ETHERNET_H
# include <net/ethernet.h>
#endif
#if HAVE_NET_IF_H
# include <net/if.h>
#endif
#if HAVE_NET_IF_ARP_H
# include <net/if_arp.h>
#endif
#if HAVE_NET_ETHERTYPES_H
# include <net/ethertypes.h>
#endif
#if HAVE_NET_IF_ETHER_H
# include <net/if_ether.h>
#endif

#include <netinet/in.h>

#include <errno.h>

#include "types.h"

#define IFNAME_MAX    64
#define PCTLERRBUFSIZ 1024

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

struct interface_info {
	char if_name[IFNAME_MAX];
};

struct arp_tables {
	struct arp_tables *pre;
	struct arp_tables *next;
	u32 paddr;
	u8 *hwaddr;
};

int write_dl(int d, void *msg, int len, struct interface_info *ifc);
int send_arp(struct interface_info *ifc, u16 type, u16 ptype,
	     struct in_addr *src_ip, struct in_addr *dst_ip,
	     u8 *src_hw, u8 *dst_hw, u8 *arp_src_hw, u8 *arp_dst_hw);
int make_ether_header(void *p, u8 *src_hw, u8 *dst_hw, u16 type);
int open_dl(struct interface_info *ifc);
int close_dl(int d);

#endif /* PCTL_H */
