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

edi_interchange_t *
edi_interchange_create(void)
{
	edi_interchange_t *p;
	
	if(-1 == edi__init())
	{
		return NULL;
	}
	p = (edi_interchange_t *) calloc(1, sizeof(edi_interchange_t));
	if(!p)
	{
		return NULL;
	}
	p->private_ = (edi_interchange_private_t *) calloc(1, sizeof(edi_interchange_private_t));
	if(!p->private_)
	{
		free(p);
		return NULL;
	}
	return p;
}

edi_segment_t *
edi_segment_create(edi_interchange_t *i, const char *tag)
{
	edi_segment_t *segp;
	
	segp = (edi_segment_t *) realloc(i->segments, sizeof(edi_segment_t) * (i->nsegments + 1));
	if(NULL == segp)
	{
		return NULL;
	}
	i->segments = segp;
	segp = &(i->segments[i->nsegments]);
	memset(segp, 0, sizeof(edi_segment_t));
	segp->interchange = i;
	if(tag)
	{
		if(NULL == edi_element_create(segp, tag))
		{
			return NULL;
		}
	}
	i->nsegments++;
	return segp;
}

edi_element_t *
edi_element_create(edi_segment_t *seg, const char *value)
{
	edi_element_t *elp;
	
	elp = (edi_element_t *) realloc(seg->elements, sizeof(edi_element_t) * (seg->nelements + 1));
	if(NULL == elp)
	{
		return NULL;
	}
	seg->elements = elp;
	elp = &(seg->elements[seg->nelements]);
	memset(elp, 0, sizeof(elp));
	elp->simple.segment = seg;
	if(value)
	{
		if(-1 == edi_element_add(elp, value))
		{
			return NULL;
		}
		if(elp == seg->elements)
		{
			seg->tag = elp->simple.value;
		}
	}
	seg->nelements++;
	return elp;
}

int
edi_element_add(edi_element_t *elp, const char *value)
{
	char *v, **vp;
	size_t vlen, vl, *lp;
	
	vlen = strlen(value);
	if(!elp->type)
	{
		elp->simple.value = edi__stringpool_alloc(elp->simple.segment->interchange, vlen);
		if(NULL == elp->simple.value)
		{
			return -1;
		}
		elp->simple.valuelen = vlen;
		elp->type = EDI_ELEMENT_SIMPLE;
		if(elp->simple.segment->elements == elp)
		{
			elp->simple.segment->tag = elp->simple.value;
		}
		return 0;
	}
	if(elp->type == EDI_ELEMENT_SIMPLE)
	{
		v = elp->simple.value;
		vl = elp->simple.valuelen;
		vp = (char **) malloc(sizeof(char *) * 2);
		lp = (size_t *) malloc(sizeof(size_t) * 2);
		if(!vp || !lp)
		{
			free(vp);
			free(lp);
			return -1;
		}
		vp[0] = elp->simple.value;
		lp[0] = elp->simple.valuelen;
		vp[1] = edi__stringpool_alloc(elp->simple.segment->interchange, vlen);
		lp[1] = vlen;
		if(!vp[1])
		{
			free(vp);
			free(lp);
			return -1;
		}
		elp->composite.values = vp;
		elp->composite.valuelens = lp;
		elp->composite.nvalues = 2;
		elp->type = EDI_ELEMENT_COMPOSITE;
		return 0;
	}
	vp = (char **) realloc(elp->composite.values, sizeof(char *) * (elp->composite.nvalues + 1));
	if(!vp)
	{
		return -1;
	}
	elp->composite.values = vp;
	lp = (size_t *) realloc(elp->composite.valuelens, sizeof(size_t) * (elp->composite.nvalues + 1));
	if(!lp)
	{
		return -1;
	}
	elp->composite.valuelens = lp;
	v = edi__stringpool_alloc(elp->composite.segment->interchange, vlen);
	if(!v)
	{
		return -1;
	}
	elp->composite.values[elp->composite.nvalues] = v;
	elp->composite.valuelens[elp->composite.nvalues] = vlen;
	elp->composite.nvalues++;
	return 0;
}

int
edi_interchange_destroy(edi_interchange_t *msg)
{
	size_t c, d, i;

	for(c = 0; c < msg->nsegments; c++)
	{
		for(d = 0; d < msg->segments[c].nelements; d++)
		{
			if(msg->segments[c].elements[d].type == EDI_ELEMENT_COMPOSITE)
			{
				for(i = 0; i < msg->segments[c].elements[d].composite.nvalues; i++)
				{
					edi__stringpool_free(msg, msg->segments[c].elements[d].composite.values[i]);
				}
				free(msg->segments[c].elements[d].composite.values);
				free(msg->segments[c].elements[d].composite.valuelens);
			}
			else
			{
				edi__stringpool_free(msg, msg->segments[c].elements[d].simple.value);
			}
		}
		free(msg->segments[c].elements);
	}
	free(msg->segments);
	edi__stringpool_destroy(msg);
	free(msg->private_);
	free(msg);
	return 0;
}

static size_t
addescaped(unsigned char *buf, size_t bufpos, size_t buflen, const char *value, size_t vlen, const edi_params_t *params)
{
	size_t n;
	char *p;

	(void) vlen;
	
	p = (char *) buf;
	n = bufpos;
	for(; *value && bufpos < buflen; value++)
	{
		if((*value == params->segment_separator ||
			*value == params->element_separator ||
			*value == params->subelement_separator ||
			*value == params->tag_separator ||
			*value == params->escape) &&
			params->escape)
		{
			*buf = params->escape;
			buf++;
			bufpos++;
			if(bufpos >= buflen) break;
		}
		*buf = (unsigned char) *value;
		buf++;
		bufpos++;
		if(bufpos >= buflen) break;
	}
	*buf = 0;
	return bufpos - n;
}

static size_t
addhdrtrailer(unsigned char *buf, size_t bufpos, size_t buflen, const char *format, edi_interchange_t *msg, const edi_params_t *params)
{
	size_t n;
	
	(void) msg;
	
	n = bufpos;
	for(; *format && bufpos < buflen; format++)
	{
		if('%' == *format)
		{
			format++;
			switch(*format)
			{
				case '_':
					if(bufpos > 0)
					{
						buf--;
						bufpos--;
					}
					continue;
				case 'S':
					*buf = params->segment_separator;
					break;
				case 'E':
					*buf = params->element_separator;
					break;
				case 's':
					*buf = params->subelement_separator;
					break;
				case 'T':
					*buf = params->tag_separator;
					break;
				case 'R':
					*buf = params->escape;
					break;
				default:
					*buf = '%';
					buf++;
					bufpos++;
					if(bufpos >= buflen)
					{
						*buf = 0;
						return bufpos - n;
					}
					*buf = *format;
			}
			buf++;
			bufpos++;
		}
		else
		{
			*buf = *format;
			buf++;
			bufpos++;
		}
	}
	*buf = 0;
	return bufpos - n;
}

size_t
edi_interchange_build(edi_interchange_t *msg, const edi_params_t *params, char *buf, size_t buflen)
{
	size_t c, d, i, n, bufpos;
	unsigned char *bp;
	const char *hdrname, *hdrtrail;
	int dohdrtrailer;
	
	if(NULL == params)
	{
		params = &edi__default_params;
	}
	bp = (unsigned char *) buf;
	bufpos = 0;
	hdrname = NULL;
	hdrtrail = NULL;
	dohdrtrailer = 0;
	if(0x0103 <= params->version)
	{
		if(NULL != params->ss_name && NULL != params->ss_trailer)
		{
			hdrname = params->ss_name;
			hdrtrail = params->ss_trailer;
		}
	}
	for(c = 0; c < msg->nsegments; c++)
	{
		for(d = 0; d < msg->segments[c].nelements; d++)
		{
			if(msg->segments[c].elements[d].type == EDI_ELEMENT_SIMPLE)
			{
				if(NULL != hdrname)
				{
					if(0 == c && 0 == d)
					{
						if(strlen(hdrname) == msg->segments[c].elements[d].simple.valuelen &&
							0 == strncmp(hdrname, msg->segments[c].elements[d].simple.value, msg->segments[c].elements[d].simple.valuelen))
						{
							dohdrtrailer = 1;
						}
						else
						{
							n = addescaped(bp, bufpos, buflen, hdrname, strlen(hdrname), params);
							bp += n;
							bufpos += n;
							if(bufpos >= buflen) break;
							n = addhdrtrailer(bp, bufpos, buflen, hdrtrail, msg, params);
							bp += n;
							bufpos += n;
							if(bufpos >= buflen) break;
						}
					}
				}
				n = addescaped(bp, bufpos, buflen, msg->segments[c].elements[d].simple.value, msg->segments[c].elements[d].simple.valuelen, params);
				bp += n;
				bufpos += n;
			}
			else
			{
				for(i = 0; i < msg->segments[c].elements[d].composite.nvalues; i++)
				{
					if(NULL != hdrname)
					{
						if(0 == c && 0 == d && 0 == i)
						{
							if(strlen(hdrname) == msg->segments[c].elements[d].simple.valuelen &&
								0 == strncmp(hdrname, msg->segments[c].elements[d].simple.value, msg->segments[c].elements[d].simple.valuelen))
							{
								dohdrtrailer = 1;
							}
							else
							{
								n = addescaped(bp, bufpos, buflen, hdrname, strlen(hdrname), params);
								bp += n;
								bufpos += n;
								if(bufpos >= buflen) break;
								n = addhdrtrailer(bp, bufpos, buflen, hdrtrail, msg, params);
								bp += n;
								bufpos += n;
								if(bufpos >= buflen) break;
							}
						}
					}
					n = addescaped(bp, bufpos, buflen, msg->segments[c].elements[d].composite.values[i], msg->segments[c].elements[d].composite.valuelens[i], params);
					bp += n;
					bufpos += n;
					if(bufpos >= buflen) break;
					if(i + 1 < msg->segments[c].elements[d].composite.nvalues)
					{
						*bp = params->subelement_separator;
						bp++;
						bufpos++;
					}
					if(bufpos >= buflen) break;
				}
			}
			if(bufpos >= buflen) break;
			if(d + 1 < msg->segments[c].nelements)
			{
				if(d == 0)
				{
					*bp = params->tag_separator;
				}
				else
				{
					*bp = params->element_separator;
				}
				bp++;
				bufpos++;
			}
			if(bufpos >= buflen) break;
		}
		if(bufpos >= buflen) break;
		if(1 == dohdrtrailer)
		{
			n = addhdrtrailer(bp, bufpos, buflen, hdrtrail, msg, params);
			bp += n;
			bufpos += n;
		}
		else
		{
			*bp = params->segment_separator;
			bp++;
			bufpos++;
		}
		if(bufpos >= buflen) break;
		dohdrtrailer = 0;
	}
	*bp = 0;
	return bufpos;
}
