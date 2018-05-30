#!/bin/sh

autoheader; aclocal; autoconf;

if test -f configure.scan; then
    rm -f configure.scan;
fi

if test -f Makefile; then
    make clean;
fi

if test -f autoscan.log; then
    rm -f autoscan.log
fi

if test -f config.h; then
    rm -f config.h
fi

if test -d autom4te.cache; then
    rm -rf autom4te.cache
fi

if test -f config.log; then
    rm -f config.log;
fi

if test -f config.status; then
    rm -f config.status;
fi

if test -f Makefile; then
    rm -f Makefile;
fi

if test -f configure.lineno; then
	rm -f configure.lineno;
fi
rm -f *~ *.o
