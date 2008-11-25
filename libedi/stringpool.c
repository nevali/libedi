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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_libedi.h"

/* Return the index of a stringpool containing at least minsize bytes available
 * buffer space, allocating one if nothing suitable exists.
 */

ssize_t
edi__stringpool_get(edi_interchange_t *msg, size_t minsize)
{
	size_t n, *lp;
	char *sp, *p, **pp;

	/* Try to allocate from the most recently-created pool */
	if(msg->private_->npools && msg->private_->stringpool[msg->private_->npools - 1])
	{
		p = msg->private_->stringpool[msg->private_->npools - 1];
		sp = msg->private_->sp[msg->private_->npools - 1];
		if((sp - p) + minsize < msg->private_->poolsize[msg->private_->npools - 1])
		{
			return msg->private_->npools - 1;
		}
	}
	/* Create a new pool */
	pp = (char **) realloc(msg->private_->stringpool, sizeof(char *) * (msg->private_->npools + 1));
	if(!pp) return -1;
	msg->private_->stringpool = pp;
	pp = (char **) realloc(msg->private_->sp, sizeof(char *) * (msg->private_->npools + 1));
	if(!pp) return -1;
	msg->private_->sp = pp;
	lp = (size_t *) realloc(msg->private_->poolsize, sizeof(size_t) * (msg->private_->npools + 1));
	if(!lp) return -1;
	msg->private_->poolsize = lp;
	n = (minsize > STRINGPOOL_BLOCKSIZE ? minsize : STRINGPOOL_BLOCKSIZE);
	sp = (char *) calloc(sizeof(char), n);
	if(!sp) return -1;
	msg->private_->stringpool[msg->private_->npools] = sp;
	msg->private_->sp[msg->private_->npools] = sp;
	msg->private_->poolsize[msg->private_->npools] = n;
	msg->private_->npools++;
	return msg->private_->npools - 1;
}

/* Allocate a buffer of length bytes from a message's stringpools */
char *
edi__stringpool_alloc(edi_interchange_t *msg, size_t length)
{
	size_t n;
	char *sp;

	n = edi__stringpool_get(msg, length);
	sp = msg->private_->sp[n];
	msg->private_->sp[n] += length;
	return sp;
}

/* Destroy all of a message's stringpools */
int
edi__stringpool_destroy(edi_interchange_t *msg)
{
	size_t c;
	
	for(c = 0; c < msg->private_->npools; c++)
	{
		free(msg->private_->stringpool[c]);
	}
	free(msg->private_->stringpool);
	free(msg->private_->sp);
	free(msg->private_->poolsize);
	msg->private_->stringpool = NULL;
	msg->private_->sp = NULL;
	msg->private_->poolsize = NULL;
	msg->private_->npools = 0;
	return 0;
}

/* Free the buffer p, if it's not contained within the message's stringpools */
int
edi__stringpool_free(edi_interchange_t *msg, char *p)
{
	size_t c;
	
	for(c = 0; c < msg->private_->npools; c++)
	{
		if(p >= msg->private_->stringpool[c] && p < msg->private_->sp[c])
		{
			return 0;
		}
	}
	free(p);
	return 0;
}
