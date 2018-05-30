/* $Id: sarp.h,v 1.3 2004/11/02 15:59:31 shj Exp $ */

#ifndef SARP_H
#define SARP_H

int verbose;

void sigdie(int n);
void cleanup(void);
int str_mac(const char *, char *, size_t);

#endif
