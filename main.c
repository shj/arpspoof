/* $Id: main.c,v 1.21 2004/11/02 16:21:09 shj Exp $ */
/* 2015/06/15 - 仕様変更 by shj<shj@newiz.jp> */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#if HAVE_NET_ETHERNET_H
# include <net/ethernet.h>
#endif
#if HAVE_NET_ETHERTYPES_H
# include <net/ethertypes.h>
#endif
# if HAVE_NET_IF_ARP_H
#  include <net/if_arp.h>
# endif

#include <netinet/in.h>
#if HAVE_NETINET_IF_ETHER_H
# include <netinet/if_ether.h>
#endif
#if HAVE_NETINET_ETHER_H
 #include <netinet/ether.h>
#endif

#include <err.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "sarp.h"
#include "pctl.h"
#include "utils.h"
#include "readconf.h"

#undef  _GNU_SOURCE
#define _GNU_SOURCE
#undef __GNU_LIBRARY__
#define __GNU_LIBRARY__
#include "getopt.h"

#define OPT_SRC_HW  1
#define OPT_DST_HW  2
#define OPT_SRC_PA  3
#define OPT_DST_PA  4
#define OPT_DEVNAME 5
#define OPT_ARP_OP  6
#define OPT_ARP_SRC_HW 7
#define OPT_ARP_DST_HW 8

/*
 * ARP Operation code from NetBSD
 */
#ifndef ARPOP_REQUEST
# define ARPOP_REQUEST    1 /* request to resolve address */
#endif
#ifndef ARPOP_REPLY
# define ARPOP_REPLY      2 /* response to previous request */
#endif
#ifndef ARPOP_REVREQUEST
# define ARPOP_REVREQUEST 3 /* request protocol address given hardware */
#endif
#ifndef ARPOP_REVREPLY
# define ARPOP_REVREPLY   4 /* response giving protocol address */
#endif
#ifndef ARPOP_INVREQUEST
# define ARPOP_INVREQUEST 8 /* request to identify peer */
#endif
#ifndef ARPOP_INVREPLY
# define ARPOP_INVREPLY   9 /* response identifying peer */
#endif

static struct option longopts[] = {
  {"srcmac",      required_argument, NULL, OPT_SRC_HW},
  {"dstmac",      required_argument, NULL, OPT_DST_HW},
  {"arp-srcip",   required_argument, NULL, OPT_SRC_PA},
  {"arp-dstip",   required_argument, NULL, OPT_DST_PA},
  {"arp-opcode" , required_argument, NULL, OPT_ARP_OP},
  {"arp-srcmac" , required_argument, NULL, OPT_ARP_SRC_HW},
  {"arp-dstmac" , required_argument, NULL, OPT_ARP_DST_HW},
  {0,0,0,0}
};

static void usage(void);
static void print_config(struct config *);
static char * print_op(int); 
static int build_config(int, char **);

static int finishup = 0;
static int flag_killproc = 0;
extern struct config *cfg;

void
usage(void)
{
	fprintf(stderr, "Usage: sarp [option(s)] [config ..]\n"
			" --srcmac <src_hw> source hardware address\n"
			" --dstmac <dst_hw> destination hardware address\n"
			" --arp-opcode <operation code> ARP operation(default request)\n"
			"   1 - request\n"
			"   2 - reply\n"
			"   3 - reverse request\n"
			"   4 - reverse reply\n"
			"   8 - identify request\n"
			"   9 - identify reply\n"
			" --arp-srcmac <src_hw> source hardware address(arp header)\n"
			" --arp-dstmac <dst_hw> destination hardware address(arp header)\n"
			" --arp-srcip <src_ip> source protocol address(ip address)\n"
			" --arp-dstip <dst_ip> destination protocol address(ip address)\n"
			" -i <interface>\n"
			" -c <n> count\n"
			" -t <ms> interval(default 500ms)\n"
			" -v Verbose\n"
			" -l loop\n"
			" -h help\n"
			);
	exit(EXIT_FAILURE);
}

static void
print_config(struct config *p)
{
	
	while (p) {
		printf("config file : %s\n", p->config);
		printf("interface   : %s\n", p->ifc.if_name);
		printf("src mac     : %s\n", ether_ntoa((struct ether_addr *)p->src_hw));
		printf("dst mac     : %s\n", ether_ntoa((struct ether_addr *)p->dst_hw));
		printf("arp src mac : %s\n", ether_ntoa((struct ether_addr *)p->arp_src_hw));
		printf("arp dst mac : %s\n", ether_ntoa((struct ether_addr *)p->arp_dst_hw));
		printf("src host    : %s\n", p->src_host);
		printf("dst host    : %s\n", p->dst_host);
		printf("arp src ip  : %s\n", inet_ntoa(p->src_ip));
		printf("arp dst ip  : %s\n", inet_ntoa(p->dst_ip));
		printf("arp op code : %d\n", p->op);
		printf("interval    : %d ms\n", p->ms);
		printf("count       : %d frame(s)\n", p->count);
		printf("\n");
		p = p->next;
	}
	
	exit(EXIT_SUCCESS);
}

static char *
print_op(int op)
{
	
	if (op == ARPOP_REQUEST)
	return "ARP request";
	if (op == ARPOP_REPLY)
	return "ARP reply";
	if (op == ARPOP_REVREQUEST)
	return "RARP request";
	if (op == ARPOP_REVREPLY)
	return "RARP reply";
	if (op == ARPOP_INVREQUEST)
	return "InARP request";
	if (op == ARPOP_INVREPLY)
	return "InARP reply";
#if 0
	if (op == ARPOP_NAK)
	return "(ATM)ARP NAK";
#endif
	else
	return "Unknown";
	
	return NULL;
}

static int
build_config(int argc, char **argv)
{
	int i, nconfig;
	
	if ((argc - optind) == 0)
		return 0;
	
	nconfig = 0;
	for (i = optind; argv[i]; ++i)  {
		fprintf(stderr, "> Loading config %s ... ", argv[i]), fflush(stderr);
		if (load_config(argv[i]) == 0) {
			fprintf(stderr, "Successful\n");
			nconfig++;
		}
	}

	return nconfig;
}

int
main(int argc, char *argv[])
{
	char *ep;
	int c;
	int nsend = 0;
	int optindex = 0;
	int optprintcfg = 0;
	int optloop = 0;
	int nconfig;
	u_long ultmp;
	struct config *tcfg, *curcfg;

	if (argc == 1)
		usage();
	
	tcfg = (struct config *)malloc(sizeof(struct config));
	if (tcfg == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	init_config(tcfg);
	while ((c = getopt_long_only(argc, argv,
		"lpvhc:t:i:f:", longopts, &optindex)) != EOF) {
	switch (c) {
		case OPT_SRC_HW:
			if (str_mac(optarg, tcfg->src_hw, sizeof(tcfg->src_hw)) < 0)
				errx(EXIT_FAILURE, "bad format hardware address");
			break;
		case OPT_DST_HW:
			if (str_mac(optarg, tcfg->dst_hw, sizeof(tcfg->dst_hw)) < 0)
			errx(EXIT_FAILURE, "bad format hardware address");
			break;
		case OPT_SRC_PA:
			tcfg->src_host = optarg;
			if (resolve(tcfg->src_host, &tcfg->src_ip) < 0)
				errx(EXIT_FAILURE, "%s: hostname lookup failure", tcfg->src_host);
			break;
		case OPT_DST_PA:
			tcfg->dst_host = optarg;
			if (resolve(tcfg->dst_host, &tcfg->dst_ip) < 0)
				errx(EXIT_FAILURE, "%s: hostname lookup failure", tcfg->dst_host);
			break;
		case OPT_DEVNAME:
			strncpy(tcfg->ifc.if_name, optarg, sizeof(tcfg->ifc.if_name));
			break;
		case OPT_ARP_OP:
 			ultmp = strtol(optarg, &ep, 0);
			if (*ep || ep == optarg || ultmp > 0xffff)
				errx(EXIT_FAILURE, "Operation code shuld be unsigned short");
			tcfg->op = ultmp;
			break;
		case OPT_ARP_SRC_HW:
			if (str_mac(optarg, tcfg->arp_src_hw, sizeof(tcfg->arp_src_hw)) < 0)
				errx(EXIT_FAILURE, "bad format hardware address");
			break;
		case OPT_ARP_DST_HW:
			if (str_mac(optarg, tcfg->arp_dst_hw, sizeof(tcfg->arp_dst_hw)) < 0)
				errx(EXIT_FAILURE, "bad format hardware address");
			break;
		case 't':
			ultmp = strtol(optarg, &ep, 0);
			if (*ep || ep == optarg || ultmp >= 1000)
				errx(EXIT_FAILURE, "Invalid interval time: %s ms", optarg);
			tcfg->ms = ultmp;
			break;
		case 'c':
			ultmp = strtol(optarg, &ep, 0);
			if (*ep || ep == optarg || ultmp > INT_MAX)
				errx(EXIT_FAILURE, "Invalid count: %s", optarg);
			tcfg->count = ultmp;
			if (tcfg->count == 0) {
				fprintf(stderr, "Warning: count parameter set to default(0=>1)\n");
				tcfg->count = 1;
			}
			break;
		case 'i':
			strncpy(tcfg->ifc.if_name, optarg, sizeof(tcfg->ifc.if_name));
			break;
		case 'v':
			verbose = 1;
			break;
		case 'p':
			optprintcfg = 1;
			break;
		case 'l':
			optloop = 1;
			break;
		case 'h':
		default:
			usage();
		}
	}
	
	cfg = NULL;
	fprintf(stderr, "> Command line parameter ... ");
	if (tcfg->ifc.if_name[0] != '\0') {
		tcfg->config = strdup("<command lind>");
		add_config_list(tcfg);
		fprintf(stderr, "Enabled\n");
	} else {
		fprintf(stderr, "Disable\n");
		tcfg = NULL;
	}

	nconfig = build_config(argc, argv);
	if (tcfg)
		nconfig++;
	
	fprintf(stderr, "> %d config loaded\n", nconfig);

	if (!nconfig)
		return -1;

	if (optprintcfg)
		print_config(cfg);
	
	signal(SIGINT, sigdie);
	signal(SIGTERM, sigdie);
	signal(SIGHUP, sigdie);
	signal(SIGSEGV, sigdie);

loop:
	curcfg = cfg;
	while (curcfg) {
		nsend = 0;
		finishup = 0;
		do {
			nsend++;
			if (curcfg->count) {
				if (curcfg->count == nsend)
					finishup = 1;
			}
			if (send_arp(&curcfg->ifc, curcfg->op, ETHERTYPE_ARP, &curcfg->src_ip, &curcfg->dst_ip,
				(u8 *)curcfg->src_hw, (u8 *)curcfg->dst_hw,
					(u8 *)curcfg->arp_src_hw, (u8 *)curcfg->arp_dst_hw) < 0) {
				perror("send_arp");
				break;
			}

			printf("%s - %s > ", curcfg->ifc.if_name, ether_ntoa((struct ether_addr *)curcfg->src_hw));
			printf("%s ", ether_ntoa((struct ether_addr *)curcfg->dst_hw));
			printf("%s(code=%d) ", print_op(curcfg->op), curcfg->op);
			printf("[%s@%s > ", ether_ntoa((struct ether_addr *)curcfg->arp_src_hw), curcfg->src_host);
			printf("%s@%s]\n", ether_ntoa((struct ether_addr *)curcfg->arp_dst_hw), curcfg->dst_host);

			usleep(curcfg->ms * 1000);
		} while (!finishup && nsend < INT_MAX);
		if (flag_killproc)
			curcfg = NULL;
		else
			curcfg = curcfg->next;
	}
	if (optloop && !flag_killproc)
		goto loop;
	cleanup();
	
	return 0;
}

int
str_mac(const char *p, char *buf, size_t len)
{
	struct ether_addr *ep;
	
	if ((ep = ether_aton(p)) == NULL)
	return -1;
	memcpy(buf, (u_char *)ep, len);
	
	return 0;
}

void
sigdie(int n)
{
	
	finishup = 1;
	flag_killproc = 1;
}

void
cleanup(void)
{
	
	free_config(cfg);
}
