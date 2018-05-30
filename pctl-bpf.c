/* $Id: pctl-bpf.c,v 1.11 2004/11/04 15:10:24 shj Exp $ */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#include <sys/time.h>
# else
#include <time.h>
#endif
#endif

#include <net/bpf.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "pctl.h"

int
open_dl(struct interface_info *ifc)
{
	int d = 0;
	int i;
	char bpfname[sizeof("/dev/bpfXX")+1];
	struct ifreq ifr;
#if defined(BIOCGHDRCMPLT) && defined(BIOCSHDRCMPLT)
	u_int spoof = 1;
#endif

for (i = 0; i < 10; i++) {
	snprintf(bpfname, sizeof(bpfname), "/dev/bpf%d", i);
	if ((d = open(bpfname, O_RDWR, 0)) > 0)
	break;
}
if (d < 0) {
	errno = ENXIO;
	return -1;
}
strncpy(ifr.ifr_name, ifc->if_name, sizeof(ifr.ifr_name));
if (ioctl(d, BIOCSETIF, &ifr) < 0)
	return -1;
#if defined(BIOCGHDRCMPLT) && defined(BIOCSHDRCMPLT)
	if (ioctl(d, BIOCSHDRCMPLT, &spoof) < 0)
	return -1;
#endif

	return d;
}

int
close_dl(int d)
{

	return close(d);
}

int
write_dl(int d, void *msg, int len, struct interface_info *ifc)
{

	return write(d, (char *)msg, len);
}
