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

/* TRADACOMS technically uses an old version of the EDI specification which
 * eventually became UN/EDIFACT. Although this original specification defines
 * a lead-in segment analogous to UNA (in this case, SCH), it is not used
 * in practice.
 *
 * TRADACOMS defaults are:
 *
 * Sub-element separator is : (colon)
 * Tag separator is = (equals)
 * Element separator is + (plus)
 * Decimal notation is . (period)
 * Escape character is ? (question mark)
 * Segment separator is ' (apostrophe)
 *
 * If TRADACOMS is being used, the first segment will be STX, and the message
 * will be terminated by END.
 */

static const edi_detector_t edi__tradacoms_detectors[] = {
	{ "STX", 0, 0, 0, 0, 0, 0, 0 },
	{ NULL, 0, 0, 0, 0, 0, 0, 0 }
};

static const edi_params_t edi__tradacoms_params = {
	EDI_VERSION,
	'\'',
	'+',
	':',
	'=',
	'?',
	"TRADACOMS",
	"STX/END,MHD/MTR",
	NULL,
	NULL
};
