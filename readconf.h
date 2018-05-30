#ifndef READCONFIG_H
#define READCONFIG_H

#include <netinet/in.h>
#include "pctl.h"

#define WHITESPACE " \t\r\n"
#define QUOTE   "\""

struct config *cfg;

struct config {
	struct interface_info ifc;
	char *config;
	char src_hw[ETH_ALEN];
	char dst_hw[ETH_ALEN];
	char arp_src_hw[ETH_ALEN];
	char arp_dst_hw[ETH_ALEN];
	char *src_host;
	char *dst_host;
	struct in_addr src_ip;
	struct in_addr dst_ip;
	int op;
	int ms;
	int count;
	struct config *next;
};

typedef enum {
	oBadOption,
	oOpcode,
	oSrcmac,
	oDstmac,
	oArpSrcmac,
	oArpDstmac,
	oArpSrcip,
	oArpDstip,
	oInterface,
	oCount,
	oInterval,
} OpCodes;

static struct {
	const char *key;
	OpCodes opcode;
} keywords[] = {
	{ "opcode", oOpcode },
	{ "srcmac", oSrcmac },
	{ "dstmac", oDstmac },
	{ "arp-srcmac", oArpSrcmac },
	{ "arp-dstmac", oArpDstmac },
	{ "arp-srcip", oArpSrcip },
	{ "arp-dstip", oArpDstip },
	{ "interface", oInterface },
	{ "count", oCount },
	{ "interval", oInterval },
	{ NULL, 0 }
};

int load_config(char *);
void init_config(struct config *);
void add_config_list(struct config *);
void free_config(struct config *);

#endif
