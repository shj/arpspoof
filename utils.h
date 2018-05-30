/* $Id: utils.h,v 1.7 2004/11/02 15:58:45 shj Exp $ */

#ifndef UTILS_H
#define UTILS_H

#if DHAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>

#include <netinet/in.h>

#include <stdarg.h>

#ifndef MAX
#define MAX(a,b) (((int)(a) > (int)(b)) ? (a) : (b))
#endif

u_short in_cksum(u_short *addr, int len);
int islocalhost(struct in_addr *addr);
int getlocaladdr(struct in_addr *src);
int resolve(const char *host, struct in_addr *addr);
char *inet_itoh(struct in_addr addr);
#ifndef HAVE_INET_NTOA
char *inet_ntoa(struct in_addr in);
#endif

#endif
