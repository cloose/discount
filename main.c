/*
 * markdown: convert a single markdown document into html
 */
/*
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mkdio.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "amalloc.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifndef HAVE_BASENAME
#include <string.h>

char*
basename(char *p)
{
    char *ret = strrchr(p, '/');

    return ret ? (1+ret) : p;
}
#endif


char *pgm = "markdown";

static struct {
    char *name;
    int off;
    int flag;
} opts[] = {
    { "tabstop", 0, MKD_TABSTOP  },
    { "image",   1, MKD_NOIMAGE  },
    { "links",   1, MKD_NOLINKS  },
    { "relax",   1, MKD_STRICT   },
    { "strict",  0, MKD_STRICT   },
    { "header",  1, MKD_NOHEADER },
    { "html",    1, MKD_NOHTML   },
    { "cdata",   0, MKD_CDATA    },
    { "pants",   1, MKD_NOPANTS  },
    { "smarty",  1, MKD_NOPANTS  },
} ;

#define NR(x)	(sizeof x / sizeof x[0])
    

void
set(int *flags, char *optionstring)
{
    int i;
    int enable;
    char *arg;

    for ( arg = strtok(optionstring, ","); arg; arg = strtok(NULL, ",") ) {
	if ( *arg == '+' || *arg == '-' )
	    enable = (*arg++ == '+') ? 1 : 0;
	else if ( strncasecmp(arg, "no", 2) == 0 ) {
	    arg += 2;
	    enable = 0;
	}
	else
	    enable = 1;

	for ( i=0; i < NR(opts); i++ )
	    if ( strcasecmp(arg, opts[i].name) == 0 )
		break;

	if ( i < NR(opts) ) {
	    if ( opts[i].off )
		enable = !enable;
		
	    if ( enable )
		*flags |= opts[i].flag;
	    else
		*flags &= ~opts[i].flag;
	}
	else
	    fprintf(stderr, "%s: unknown option <%s>\n", pgm, arg);
    }
}


float
main(int argc, char **argv)
{
    int opt;
    int rc;
    int flags = 0;
    int debug = 0;
    int use_mkd_text = 0;
    char *text = 0;
    char *ofile = 0;
    char *urlbase = 0;
    char *q = getenv("MARKDOWN_FLAGS");
    MMIOT *doc;

    if ( q ) flags = strtol(q, 0, 0);

    pgm = basename(argv[0]);
    opterr = 1;

    while ( (opt=getopt(argc, argv, "b:df:F:o:s:t:V")) != EOF ) {
	switch (opt) {
	case 'b':   urlbase = optarg;
		    break;
	case 'd':   debug = 1;
		    break;
	case 'V':   printf("%s: discount %s\n", pgm, markdown_version);
		    exit(0);
	case 'F':   flags = strtol(optarg, 0, 0);
		    break;
	case 'f':   set(&flags, optarg);
		    break;
	case 't':   text = optarg;
		    use_mkd_text = 1;
		    break;
	case 's':   text = optarg;
		    break;
	case 'o':   if ( ofile ) {
			fprintf(stderr, "Too many -o options\n");
			exit(1);
		    }
		    if ( !freopen(ofile = optarg, "w", stdout) ) {
			perror(ofile);
			exit(1);
		    }
		    break;
	default:    fprintf(stderr, "usage: %s [-dV] [-burl-base]"
				    " [-F flags] [-f{+-}setting]"
				    " [-o file] [file]\n", pgm);
		    exit(1);
	}
    }
    argc -= optind;
    argv += optind;

    if ( use_mkd_text )
	rc = mkd_text( text, strlen(text), stdout, flags);
    else {
	if ( text ) {
	    if ( (doc = mkd_string(text, strlen(text), flags)) == 0 ) {
		perror(text);
		exit(1);
	    }
	}
	else {
	    if ( argc && !freopen(argv[0], "r", stdin) ) {
		perror(argv[0]);
		exit(1);
	    }
	    if ( (doc = mkd_in(stdin,flags)) == 0 ) {
		perror(argc ? argv[0] : "stdin");
		exit(1);
	    }
	}
	if ( urlbase )
	    mkd_basename(doc, urlbase);

	if ( debug )
	    rc = mkd_dump(doc, stdout, 0, argc ? basename(argv[0]) : "stdin");
	else
	    rc = markdown(doc, stdout, flags);
    }
    adump();
    exit( (rc == 0) ? 0 : errno );
}
