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

#include "edifact.h"
#include "tradacoms.h"
#include "x12.h"

static size_t ndetectparams;
static edi_regparams_t **detectparams;
#ifdef LIBEDI_USE_PTHREAD
static pthread_mutex_t detectlock = PTHREAD_MUTEX_INITIALIZER;
#endif


static inline void edi__detect_lock(void);
static inline void edi__detect_unlock(void);
static int edi__detect_regset(const char *name, const edi_params_t *src, const edi_detector_t *detectors);
static edi_regparams_t *edi__detect_register_params(const char *name, const edi_params_t *params);
static int edi__detect_register(edi_regparams_t *params, const edi_detector_t *detector);
static int edi__detect_regset(const char *name, const edi_params_t *src, const edi_detector_t *detectors);
static int edi__detect_rp_init(edi_regparams_t *dest, const edi_params_t *params);
static int edi__detect_rp_cleanup(edi_regparams_t *rp);

int
edi__detect_init(void)
{
	edi__detect_regset("UN/EDIFACT", &edi_edifact_params, edi__edifact_detectors);
	edi__detect_regset("TRADACOMS", &edi__tradacoms_params, edi__tradacoms_detectors);
	edi__detect_regset("ANSI X12", &edi__x12_params, edi__x12_detectors);
	return 0;
}

/* Register a set of parameters under a given name (which may be NULL) */
edi_regparams_t *
edi_params_register(const char *name, const edi_params_t *params)
{
	if(-1 == edi__init())
	{
		return NULL;
	}
	return edi__detect_register_params(name, params);
}

const edi_regparams_t *
edi_detect_get(const char *name)
{
	size_t c;
	const edi_regparams_t *p;
	
	if(-1 == edi__init())
	{
		return NULL;
	}
	p = NULL;
	edi__detect_lock();
	for(c = 0; c < ndetectparams; c++)
	{
		if(detectparams[c]->name[0] && 0 == strcmp(name, detectparams[c]->name))
		{
			p = detectparams[c];
			break;
		}
	}
	edi__detect_unlock();
	return p;
}

const edi_params_t *
edi_detect_get_params(const char *name)
{
	const edi_regparams_t *p;
	
	if(NULL == (p = edi_detect_get(name)))
	{
		return NULL;
	}
	return &(p->params);
}

/* If we do detection, fill in @params, else leave it alone. If the matched
 * detector specified skipbytes, store this in @skip.
 */

int
edi__detect(edi_parser_t *parser, const char *message, edi_params_t *params, size_t *skip)
{
	size_t len, c, n;
	edi_params_t *p;
	edi_detector_t *d;
	
	(void) parser;
	
	len = strlen(message);
	edi__detect_lock();
	for(c = 0; c < ndetectparams; c++)
	{
		p = &(detectparams[c]->params);
		for(n = 0; n < detectparams[c]->ndetectors; n++)
		{
			d = &(detectparams[c]->detectors[n]);
			if(d->position > len || d->position + strlen(d->detectstr) > len || d->skipbytes > len)
			{
				continue;
			}
			if(0 == strncmp(d->detectstr, message, strlen(d->detectstr)))
			{
				/* We have a match */
				*skip = d->skipbytes;
				memset(params, 0, sizeof(params));
				params->version = EDI_VERSION;
				params->segment_separator = p->segment_separator;
				params->element_separator = p->element_separator;
				params->subelement_separator = p->subelement_separator;
				params->tag_separator = p->tag_separator;
				params->escape = p->escape;
				if(d->segment_separator_pos)
				{
					params->segment_separator = message[d->position + d->segment_separator_pos];
/*					fprintf(stderr, "segment separator is %c\n", params->segment_separator); */
				}
				if(d->element_separator_pos)
				{
					params->element_separator = message[d->position + d->element_separator_pos];
/*					fprintf(stderr, "element separator is %c\n", params->element_separator); */
				}
				if(d->subelement_separator_pos)
				{
					params->subelement_separator = message[d->position + d->subelement_separator_pos];
/*					fprintf(stderr, "sub-element separator is %c\n", params->subelement_separator); */
					
				}
				if(d->tag_separator_pos)
				{
					params->tag_separator = message[d->position + d->tag_separator_pos];
/*					fprintf(stderr, "tag separator is %c\n", params->tag_separator); */
					
				}
				if(d->escape_pos)
				{
					params->escape = message[d->position + d->escape_pos];
/*					fprintf(stderr, "release is %c\n", params->escape); */
				}
				edi__detect_unlock();
				return 0;
			}
		}
	}
	/* No match */
	edi__detect_unlock();
	return 0;
}

static inline void
edi__detect_lock(void)
{
#ifdef LIBEDI_USE_PTHREAD
	pthread_mutex_lock(&detectlock);
#endif
}

static inline void
edi__detect_unlock(void)
{
#ifdef LIBEDI_USE_PTHREAD
	pthread_mutex_unlock(&detectlock);
#endif
}

static edi_regparams_t *
edi__detect_register_params(const char *name, const edi_params_t *params)
{
	size_t c;
	char tbuf[32];
	edi_regparams_t *p, **q;
	
	if(NULL != name)
	{
		strncpy(tbuf, name, sizeof(tbuf));
		tbuf[sizeof(tbuf) - 1] = 0;
	}
	edi__detect_lock();
	if(NULL != name)
	{
		for(c = 0; c < ndetectparams; c++)
		{
			if(0 != detectparams[c]->name[0])
			{
				if(0 == strcmp(detectparams[c]->name, tbuf))
				{
					break;
				}
			}
		}
		if(c < ndetectparams)
		{
			edi__detect_rp_cleanup(detectparams[c]);
			edi__detect_rp_init(detectparams[c], params);
			p = detectparams[c];
			edi__detect_unlock();
			return p;
		}
	}
	if(NULL == (p = (edi_regparams_t *) calloc(1, sizeof(edi_regparams_t))))
	{
		edi__detect_unlock();
		return NULL;
	}
	if(NULL == (q = (edi_regparams_t **) realloc(detectparams, sizeof(edi_regparams_t *) * (ndetectparams + 1))))
	{
		edi__detect_unlock();
		free(q);
		return NULL;
	}
	detectparams = q;
	detectparams[ndetectparams] = p;
	ndetectparams++;
	edi__detect_rp_init(p, params);
	if(NULL != name)
	{
		strcpy(p->name, tbuf);
	}
	edi__detect_unlock();
	return p;
}

static int
edi__detect_regset(const char *name, const edi_params_t *src, const edi_detector_t *detectors)
{
	edi_regparams_t *p;
	size_t c;
	
	p = edi__detect_register_params(name, src);
	if(NULL != detectors)
	{
		for(c = 0; NULL != detectors[c].detectstr; c++)
		{
			edi__detect_register(p, &(detectors[c]));
		}
	}
	return 0;
}

static int
edi__detect_register(edi_regparams_t *params, const edi_detector_t *detector)
{
	edi_detector_t *q;
	size_t c;
	
	edi__detect_lock();
	q = NULL;
	if(params->ndetectors + 1 > params->nalloc)
	{
		if(NULL == (q = (edi_detector_t *) realloc(params->detectors, sizeof(edi_detector_t) * (params->ndetectors + 1))))
		{
			edi__detect_unlock();
			return -1;
		}
		params->detectors = q;
		params->nalloc = params->ndetectors + 1;
	}
	else
	{
		for(c = 0; c < params->ndetectors; c++)
		{
			if(NULL == params->detectors[c].detectstr)
			{
				q = &(params->detectors[c]);
				break;
			}
		}
	}
	q = &(params->detectors[params->ndetectors]);
	memcpy(q, detector, sizeof(edi_detector_t));
	if(NULL == (q->detectstr = strdup(detector->detectstr)))
	{
		edi__detect_unlock();
		return -1;
	}
	params->ndetectors++;
	edi__detect_unlock();
	return 0;
}

static int
edi__detect_params_copy(edi_params_t *dest, const edi_params_t *src)
{
	dest->version = EDI_VERSION;
	dest->segment_separator = '\'';
	dest->element_separator = '+';
	dest->subelement_separator = ':';
	dest->tag_separator = '+';
	dest->escape = '?';
	if(src->version >= 0x0100)
	{
		dest->segment_separator = src->segment_separator;
		dest->element_separator = src->element_separator;
		dest->subelement_separator = src->subelement_separator;
		dest->tag_separator = src->tag_separator;
		dest->escape = src->escape;
	}
	if(src->version >= 0x0102)
	{
		if(NULL != src->xml_root_node)
		{
			if(NULL == (dest->xml_root_node = strdup(src->xml_root_node)))
			{
				return -1;
			}
		}
		if(NULL != src->containers)
		{
			if(NULL == (dest->containers = strdup(src->containers)))
			{
				return -1;
			}
		}
	}
	if(src->version >= 0x0103)
	{
		if(NULL != src->ss_name)
		{
			if(NULL == (dest->ss_name = strdup(src->ss_name)))
			{
				return -1;
			}
		}
		if(NULL != src->ss_trailer)
		{
			if(NULL == (dest->ss_trailer = strdup(src->ss_trailer)))
			{
				return -1;
			}
		}
		
	}
	return 0;
}

static int 
edi__detect_rp_init(edi_regparams_t *dest, const edi_params_t *params)
{
	memset(dest, 0, sizeof(edi_regparams_t));
	dest->params.version = EDI_VERSION;
	edi__detect_params_copy(&dest->params, params);
	/* Most in-use EDI flavours need two detectors */
	dest->nalloc = 2;
	if(NULL == (dest->detectors = (edi_detector_t *) calloc(dest->nalloc, sizeof(edi_detector_t))))
	{
		return -1;
	}
	return 0;
}

static int
edi__detect_rp_cleanup(edi_regparams_t *rp)
{
	size_t c;
	
	for(c = 0; c < rp->ndetectors; c++)
	{
		free((char *) rp->detectors[c].detectstr);
		rp->detectors[c].detectstr = NULL;
	}
	free((char *) (rp->params.xml_root_node));
	rp->params.xml_root_node = NULL;
	free((char *) (rp->params.containers));
	rp->params.containers = NULL;
	return 0;
}
