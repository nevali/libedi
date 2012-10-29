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

static int edi__parser_init(edi_parser_t *parser, const edi_params_t *params);
static size_t memcpyescape(char *dest, const char *src, int escape, size_t len);

edi_parser_t *
edi_parser_create(const edi_params_t *params)
{
	edi_parser_t *p;
	
	if(-1 == edi__init())
	{
		return NULL;
	}
	if(NULL == (p = (edi_parser_t *) calloc(1, sizeof(edi_parser_t))))
	{
		return NULL;
	}
	if(NULL == params)
	{
		/* Provide some sensible defaults, just in case */
		p->sep_seg = '\'';
		p->sep_data = '+';
		p->sep_sub = ':';
		p->sep_tag = '+';
		p->escape = '?';
		p->detect = 1;
	}
	else if(-1 == edi__parser_init(p, params))
	{
		free(p);
		return NULL;
	}
	return p;
}

int 
edi_parser_destroy(edi_parser_t *parser)
{
	free(parser);
	return 0;
}

edi_interchange_t *
edi_parser_parse(edi_parser_t *oparser, const char *message)
{
	int e;
	const char *ts;
	char *value, **vp;
	size_t len, *lp;
	edi_interchange_t *p;
	edi_segment_t *seg, *segp;
	edi_element_t *el, *elp;
	size_t skip, segalloc, elalloc;
	int newel;
	edi_parser_t *parser, staticparser;
	edi_params_t params;
	const char * msg_start = message;
	
	parser = oparser;
	if(1 == oparser->detect)
	{
		/* Attempt auto-detection; if it succeeds, and a params
		 * struct is returned, construct a temporary parser based upon
		 * it and skip the necessary number of bytes.
		 */
		params.version = 0;
		if(-1 == edi__detect(oparser, message, &params, &skip))
		{
			return NULL;
		}
		if(0 != params.version)
		{
			if(-1 == edi__parser_init(&staticparser, &params))
			{
				oparser->error = EDI_ERR_SYSTEM;
				return NULL;
			}
			parser = &staticparser;
		}
		message += skip;
	}
	parser->error = EDI_ERR_NONE;
	if(NULL == (p = edi_interchange_create()))
	{
		parser->error = EDI_ERR_SYSTEM;
		return NULL;
	}
	segalloc = 0;
	if(!message || !*message)
	{
		parser->error = EDI_ERR_EMPTY;
		return p;
	}
	/* We know that the buffer required to hold the values resulting from 
	 * parsing won't exceed the size of the message in the first place,
	 * so create a stringpool of that size first.
	 */
	edi__stringpool_get(p, strlen(message) + 1);
	while(message && *message)
	{
#if 1 /* Can't find where is it documented, but newlines after segment separator is quite common */
		while(*message == 0x0D || *message == 0x0A)
			++message;
		if(! *message)
			break;
#endif
		if(p->nsegments + 1 > segalloc)
		{
			segp = (edi_segment_t *) realloc(p->segments, sizeof(edi_segment_t) * (segalloc + SEG_BLOCKSIZE));
			if(NULL == segp)
			{
				parser->error = EDI_ERR_SYSTEM;
				message = NULL;
				break;
			}
			p->segments = segp;
			segalloc += SEG_BLOCKSIZE;
		}
		seg = &(p->segments[p->nsegments]);
		p->nsegments++;
		memset(seg, 0, sizeof(edi_segment_t));
		seg->interchange = p;
		seg->offset = message - msg_start;
		elalloc = 0;
		newel = 1;
		el = NULL;
		/* Loop the data elements */
		while(*message && *message != parser->sep_seg)
		{
			if(newel)
			{
				if(seg->nelements + 1 > elalloc)
				{
					elp = (edi_element_t *) realloc(seg->elements, sizeof(edi_element_t) * (elalloc + ELEMENT_BLOCKSIZE));
					if(NULL == elp)
					{
						parser->error = EDI_ERR_SYSTEM;
						message = NULL;
						break;
					}
					seg->elements = elp;
					elalloc += 8;
				}
				el = &(seg->elements[seg->nelements]);
				seg->nelements++;
				memset(el, 0, sizeof(edi_element_t));
				el->simple.segment = seg;
				newel = 0;
			}
			ts = message;
			e = 0;
			while(*message)
			{
				if(e)
				{
					message++;
					e = 0;
					continue;
				}
				if(parser->escape && *message == parser->escape)
				{
					e = 1;
					message++;
					continue;
				}
				if(*message == parser->sep_sub || 
					(seg->elements != el && *message == parser->sep_data) ||
					(seg->elements == el && *message == parser->sep_tag) ||
					*message == parser->sep_seg)
				{
					break;
				}
				message++;
			}
			value = edi__stringpool_alloc(p, message - ts + 1);
			len = memcpyescape(value, ts, parser->escape, message - ts);
			value[len] = 0;
			if(el->type == EDI_ELEMENT_COMPOSITE || *message == parser->sep_sub)
			{
				el->type = EDI_ELEMENT_COMPOSITE;
				vp = (char **) realloc(el->composite.values, sizeof(char *) * (el->composite.nvalues + 2));
				if(NULL == vp)
				{
					parser->error = EDI_ERR_SYSTEM;
					message = NULL;
					break;
				}
				el->composite.values = vp;
				lp = (size_t *) realloc(el->composite.valuelens, sizeof(size_t) * (el->composite.nvalues + 2));
				if(NULL == lp)
				{
					parser->error = EDI_ERR_SYSTEM;
					message = NULL;
					break;
				}
				el->composite.valuelens = lp;
				vp[el->composite.nvalues] = value;
				lp[el->composite.nvalues] = len;
				el->composite.nvalues++;
				vp[el->composite.nvalues] = NULL;
				lp[el->composite.nvalues] = 0;
				if(el == seg->elements && el->composite.nvalues == 1)
				{
					seg->tag = value;
				}
			}
			else
			{
				el->type = EDI_ELEMENT_SIMPLE;
				el->simple.value = value;
				el->simple.valuelen = len;
				if(el == seg->elements)
				{
					seg->tag = value;
				}
			}
			if(!*message || *message == parser->sep_seg)
			{
				break;
			}
			if(*message == parser->sep_data || *message == parser->sep_tag)
			{
				newel = 1;
			}
			/* Move past the tag, data element or sub-element separator */
			message++;
		}
		if(NULL == message)
		{
			break;
		}
		if(!*message)
		{
			parser->error = EDI_ERR_UNTERMINATED;
			break;
		}
		/* Move past the segment separator */
		message++;
	}
	return p;
}

int
edi_parser_error(edi_parser_t *p)
{
	return p->error;
}

static int
edi__parser_init(edi_parser_t *p, const edi_params_t *params)
{
	memset(p, 0, sizeof(edi_parser_t));
	p->detect = 1;
	p->error = EDI_ERR_NONE;
	if(NULL == params)
	{
		params = &edi__default_params;
	}
	if(params->version >= 0x0100)
	{
		p->sep_seg = params->segment_separator;
		p->sep_data = params->element_separator;
		p->sep_sub = params->subelement_separator;
		p->sep_tag = params->tag_separator;
		p->escape = params->escape;
	}
	return 0;
}

/* Copy from src to dest, removing an escape character, returning the number of
 * bytes copied into dest.
 */
static size_t
memcpyescape(char *dest, const char *src, int escape, size_t len)
{
	size_t c;
	int e;
	
	e = 0;
	c = 0;
	while(len)
	{
		if(e)
		{
			*dest = *src;
			dest++;
			src++;
			len--;
			c++;
			e = 0;
		}
		else if(escape && *src == escape)
		{
			e = 1;
			src++;
			len--;
		}
		else
		{
			*dest = *src;
			dest++;
			src++;
			c++;
			len--;
		}
	}
	return c;
}
