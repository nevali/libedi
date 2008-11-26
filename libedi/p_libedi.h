/* @(#) $Id$ */

/*
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Mo McRoberts.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the author(s) of this software may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * AUTHORS OF THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef P_LIBEDI_H_
# define P_LIBEDI_H_                   1

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# ifdef HAVE_PTHREAD_H
#  include <pthread.h>
# endif

# define LIBEDI_EXPORTS                1
# ifdef PIC
#  define LIBEDI_SHARED                1
# endif

# include "libedi.h"

struct edi_parser_struct
{
	int error; /* Error status */
	int sep_seg; /* Segment separator */
	int sep_data; /* Data element separator */
	int sep_sub; /* Sub-element separator */
	int sep_tag; /* Tag delimiter */
	int escape; /* Escape (release) character */
	int detect; /* If 1, allow auto-detection */
	char *root; /* Root element to use by default */
};

struct edi_interchange_private_struct
{
	char **stringpool;
	char **sp;
	size_t *poolsize;
	size_t npools;
};

struct edi_regparams_struct
{
	char name[32];
	edi_params_t params;
	size_t ndetectors;
	size_t nalloc;
	edi_detector_t *detectors;
};

# define STRINGPOOL_BLOCKSIZE          512
# define SEG_BLOCKSIZE                 8
# define ELEMENT_BLOCKSIZE             8

extern const edi_params_t edi__default_params;

int edi__init(void);

ssize_t edi__stringpool_get(edi_interchange_t *msg, size_t minsize);
char *edi__stringpool_alloc(edi_interchange_t *msg, size_t length);
int edi__stringpool_free(edi_interchange_t *msg, char *p);
int edi__stringpool_destroy(edi_interchange_t *msg);

int edi__detect_init(void);
int edi__detect(edi_parser_t *parser, const char *message, edi_params_t *params, size_t *skip);

#endif /* !P_LIBEDI_H_ */
