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

#ifndef LIBEDI_H_
# define LIBEDI_H_                     1

# include <sys/types.h>

# define EDI_VERSION                   0x0103

# define EDI_ELEMENT_SIMPLE            'S'
# define EDI_ELEMENT_COMPOSITE         'C'

# define EDI_ERR_NONE                  0      /* No error occurred */
# define EDI_ERR_SYSTEM                1      /* See errno for further information */
# define EDI_ERR_UNTERMINATED          2      /* Parsing ended before the segment was terminated */
# define EDI_ERR_EMPTY                 3      /* Parsing ended because the message was empty */

typedef struct edi_parser_struct edi_parser_t;
typedef struct edi_detector_struct edi_detector_t;
typedef struct edi_params_struct edi_params_t;
typedef struct edi_regparams_struct edi_regparams_t;
typedef struct edi_interchange_struct edi_interchange_t;
typedef struct edi_segment_struct edi_segment_t;
typedef union edi_element_struct edi_element_t;
typedef struct edi_interchange_private_struct edi_interchange_private_t;

/* Detector specifiers */
struct edi_detector_struct
{
	const char *detectstr;
	size_t position;
	size_t skipbytes;
	/* Separator positions, relative to <position>. Set to zero to indicate
	 * that defaults should be used.
	 */
	size_t segment_separator_pos;
	size_t element_separator_pos;
	size_t subelement_separator_pos;
	size_t tag_separator_pos;
	size_t escape_pos;
};

/* Parser parameters structure. Always set version to EDI_VERSION to indicate
 * which version of the structure you've compiled against.
 */
struct edi_params_struct
{
	int version;
	unsigned char segment_separator;
	unsigned char element_separator;
	unsigned char subelement_separator;
	unsigned char tag_separator;
	unsigned char escape;
	/* The tag used for the root node, which replaces the start/end segments
	 * in XML output.
	 */
	const char *xml_root_node;
	/* List of container segments, in the form START/END,START/END,... */
	const char *containers;
	/* Interchange header name (e.g., UNA, ISA) */
	const char *ss_name;
	/* Interchange header trailer; replaces the segment separator. Format:
	  %_ = backspace (used by X12)
	  %S = segment separator
	  %E = element separator
	  %s = subelement separator
	  %T = tag separator
	  %R = release (escape)
	*/
	const char *ss_trailer;
};

/* An EDI interchange (message), consisting of a number of segments */
struct edi_interchange_struct
{
	edi_interchange_private_t *private_;
	edi_segment_t *segments;
	size_t nsegments;
};

/* An EDI segment, which consists of a number of elements. The first element
 * usually constitutes the element "tag".
 */
struct edi_segment_struct
{
	edi_interchange_t *interchange;
	edi_element_t *elements;
	size_t nelements;
	char *tag;
	size_t offset;
};

/* An EDI data element. This comes in one of two flavours - EDI_ELEMENT_SIMPLE,
 * where there is a single data value, and EDI_ELEMENT_COMPOSITE, where there
 * are multiple data values.
 */
union edi_element_struct
{
	char type;
	struct
	{
		char type;
		edi_segment_t *segment;
		char *value;
		size_t valuelen;
	} simple;
	struct
	{
		char type;
		edi_segment_t *segment;
		char **values;
		size_t *valuelens;
		size_t nvalues;
	} composite;
};

# undef EXTERNC_
# if defined(__cplusplus)
#  define EXTERNC_                     extern "C"
# else
#  define EXTERNC_                     /* */
# endif

# undef DLLEXPORT_
# undef DLLIMPORT_
# if defined(_WIN32) && defined(LIBEDI_SHARED)
#   if defined(__GNUC__)
#    define DLLEXPORT_                 __attribute__ ((dllexport))
#    define DLLIMPORT_                 __attribute__ ((dllimport))
#   else
#    define DLLEXPORT_                 __declspec(dllexport)
#    define DLLEXPORT_                 __declspec(dllexport)
#   endif
# else
#  define DLLEXPORT_                   /* */
#  define DLLIMPORT_                   /* */
# endif

# undef PUBLISHED
# undef IMPEXP_
# if defined(LIBEDI_EXPORTS)
#  define PUBLISHED                    EXTERNC_ DLLEXPORT_
#  define IMPEXP_                      DLLEXPORT_
# else
#  define PUBLISHED                    EXTERNC_ DLLIMPORT_
#  define IMPEXP_                      DLLIMPORT_
# endif

/* Core EDI parser */

PUBLISHED edi_parser_t *edi_parser_create(const edi_params_t *params);
PUBLISHED int edi_parser_destroy(edi_parser_t *parser);
PUBLISHED edi_interchange_t *edi_parser_parse(edi_parser_t *parser, const char *message);
PUBLISHED int edi_parser_error(edi_parser_t *p);

/* EDI message building */

PUBLISHED edi_interchange_t *edi_interchange_create(void);
PUBLISHED int edi_interchange_destroy(edi_interchange_t *interchange);
PUBLISHED size_t edi_interchange_build(edi_interchange_t *msg, const edi_params_t *params, char *buf, size_t buflen);
	
PUBLISHED edi_segment_t *edi_segment_create(edi_interchange_t *interchange, const char *tag);

PUBLISHED edi_element_t *edi_element_create(edi_segment_t *seg, const char *value);
PUBLISHED int edi_element_add(edi_element_t *el, const char *value);

/* Detection */
PUBLISHED edi_regparams_t *edi_params_register(const char *name, const edi_params_t *params);
PUBLISHED const edi_regparams_t *edi_detect_get(const char *name);
PUBLISHED const edi_params_t *edi_detect_get_params(const char *name);

/* Presets */

extern IMPEXP_ const edi_params_t edi_edifact_params;

#endif /* !LIBEDI_H_ */
