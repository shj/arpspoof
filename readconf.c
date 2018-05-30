/*
 * readconf.c
 * OpenSSHからコードを拝借
 * 2015/06/19
 * by shj<shj@newiz.jp>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include "readconf.h"
#include "sarp.h"
#include "utils.h"

extern struct config *cfg;

static char *
strdelim(char **s)
{
	char *old;
	int wspace = 0;

	if (*s == NULL)
		return NULL;

	old = *s;

	*s = strpbrk(*s, WHITESPACE QUOTE "=");
	if (*s == NULL)
		return (old);

	if (*s[0] == '\"') {
		memmove(*s, *s + 1, strlen(*s)); /* move nul too */
		/* Find matching quote */
		if ((*s = strpbrk(*s, QUOTE)) == NULL) {
				return (NULL);          /* no matching quote */
		} else {
			*s[0] = '\0';
			*s += strspn(*s + 1, WHITESPACE) + 1;
			return (old);
		}
	}

	/* Allow only one '=' to be skipped */
	if (*s[0] == '=')
		wspace = 1;
		*s[0] = '\0';

	/* Skip any extra whitespace after first token */
	*s += strspn(*s + 1, WHITESPACE) + 1;
	if (*s[0] == '=' && !wspace)
		*s += strspn(*s + 1, WHITESPACE) + 1;

	return (old);
}

static OpCodes
parse_token(const char *cp, const char *configfile, int linenum)
{
	int i;

	for (i = 0; keywords[i].key; i++)
		if (strcmp(cp, keywords[i].key) == 0)
			return keywords[i].opcode;
	fprintf(stderr, "\n%s: line %d: Bad configuration option: %s", configfile, linenum, cp);

	return oBadOption;
}

/*
 * 設定エラーの数を返す
 */
static int
process_config_line(const char *configfile, int linenum, char *line, struct config *newcfg)
{
	char  *s, *arg, *keyword;
	size_t len;
	int opcode;
	int badopt = 0;
	
	for (len = strlen(line) - 1; len > 0; len--) {
		if (strchr(WHITESPACE, line[len]) == NULL)
			break;
		line[len] = '\0';
	}

	s = line;
	if ((keyword = strdelim(&s)) == NULL)
		return 0;
	if (*keyword == '\0')
		keyword = strdelim(&s);
	if (keyword == NULL || !*keyword || *keyword == '\n' || *keyword == '#')
		return 0;

	opcode = parse_token(keyword, configfile, linenum);

	switch (opcode) {
	case oOpcode:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			newcfg->op = 1;
		else
			newcfg->op = atoi(arg);
		break;
	case oSrcmac:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else {
			if (str_mac(arg, newcfg->src_hw, sizeof(newcfg->src_hw)) < 0) {
				badopt++;
				fprintf(stderr, "\nsrcmac: bad format hardware address");
			}
		}
		break;
	case oDstmac:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else {
			if (str_mac(arg, newcfg->dst_hw, sizeof(newcfg->dst_hw)) < 0) {
				badopt++;
				fprintf(stderr, "\ndstmac: bad format hardware address");
			}
		}
		break;
	case oArpSrcmac:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else  {
			if (str_mac(arg, newcfg->arp_src_hw, sizeof(newcfg->arp_src_hw)) < 0) {
				badopt++;
				fprintf(stderr, "\narp-srcmac: bad format hardware address");
			}
		}
		break;
	case oArpDstmac:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else {
			if (str_mac(arg, newcfg->arp_dst_hw, sizeof(newcfg->arp_dst_hw)) < 0) {
				badopt++;
				fprintf(stderr, "\narp-dstmac: bad format hardware address");
			}
		}
		break;
	case oArpSrcip:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else {
			if (resolve(arg, &newcfg->src_ip) < 0) {
				badopt++;
				fprintf(stderr, "\narp-srcip: %s: hostname lookup failure", arg);
			} else {
				newcfg->src_host = strdup(arg);
			}
		}
		break;
	case oArpDstip:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else {
			if (resolve(arg, &newcfg->dst_ip) < 0) {
				badopt++;
				fprintf(stderr, "\narp-dstip: %s: hostname lookup failure", arg);
			} else {
				newcfg->dst_host = strdup(arg);
			}
		}
		break;
	case oInterface:
		arg = strdelim(&s);
		if (!arg || *arg == '\0') {
			badopt++;
			fprintf(stderr, "\ninterface: need interface name");
		} else {
			strncpy(newcfg->ifc.if_name, arg, sizeof(newcfg->ifc.if_name));
		}
		break;
	case oCount:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else
			newcfg->count = atoi(arg);
		break;
	case oInterval:
		arg = strdelim(&s);
		if (!arg || *arg == '\0')
			;
		else
			newcfg->ms = atoi(arg);
		break;
	default:
		break;
	}
	
	return badopt;
}

void
add_config_list(struct config *add)
{

	add->next = cfg;
	cfg = add;
}

/*
 * コンフィグの読み込みに成功した場合は0、失敗した場合は-1を返す
 */
int
load_config(char *configfile)
{
	FILE *fp;
	char line[BUFSIZ];
	int linenum;
	int bad_options = 0;
	struct config *newcfg;

	fp = fopen(configfile, "r");
	if (fp == NULL) {
		perror(configfile);
		return -1;
	}
	
	newcfg = (struct config *)malloc(sizeof(struct config));
	if (newcfg == NULL) {
		perror("malloc");
		return -1;
	}
	init_config(newcfg);
	
	linenum = 0;
	while (fgets(line, sizeof(line), fp)) {
		linenum++;
		if (process_config_line(configfile, linenum, line, newcfg) != 0)
		bad_options++;
	}
	fclose(fp);
	if (bad_options > 0) {
		free(newcfg);
		fprintf(stderr, "\n%s: termination, %d bad configuration options\n",
			configfile, bad_options);
		return -1;
	} else {
		newcfg->config = strdup(configfile);
		add_config_list(newcfg);
	}

	return 0;
}

void
free_config(struct config *p)
{
	struct config *pnext;
	
	while (p) {
		free(p->config); p->config = NULL;
		pnext = p->next;
		free(p); p = NULL;
		p = pnext;
	}
}

void
init_config(struct config *c)
{

	memset(c, 0, sizeof(struct config));
	c->src_host = "0.0.0.0";
	c->dst_host = "0.0.0.0";
	c->ms = 1000;
	c->op = 1;
	c->count = 1;
}
