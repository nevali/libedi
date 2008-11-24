#include <stdio.h>
#include <string.h>

#include "libedi.h"

const char *msg1 = 
	"UNB+IATB:1+6XPPC+LHPPC+940101:0950+1'"
	"UNH+1+PAORES:93:1:IA'"
	"MSG+1:45'"
	"IFT+3+XYZCOMPANY AVAILABILITY'"
	"ERC+A7V:1:AMD'"
	"IFT+3+NO MORE FLIGHTS'"
	"ODI'"
	"TVL+240493:1000::1220+FRA+JFK+DL+400+C'"
	"PDI++C:3+Y::3+F::1'"
	"APD+74C:0:::6++++++6X'"
	"TVL+240493:1740::2030+JFK+MIA+DL+081+C'"
	"PDI++C:4'"
	"APD+EM2:0:1630::6+++++++DA'"
	"UNT+13+1'"
	"UNZ+1+1'";

void
dump_element(size_t index, edi_element_t *el)
{
	size_t c;
	
	fprintf(stderr, "  %3u %c ", (unsigned int) index, el->type);
	if(el->type == EDI_ELEMENT_SIMPLE)
	{
		fprintf(stderr, " '%s'\n", el->simple.value);
	}
	else
	{
		fprintf(stderr, " (");
		for(c = 0; c < el->composite.nvalues; c++)
		{
			fprintf(stderr, "'%s'", el->composite.values[c]);
			if(c + 1 < el->composite.nvalues)
			{
				fprintf(stderr, ", ");
			}
		}
		fprintf(stderr, ")\n");
	}
}

void
dump_segment(size_t index, edi_segment_t *s)
{
	size_t c;
	
	fprintf(stderr, "Segment %u [%s]:\n", (unsigned int) index, s->tag);
	for(c = 0; c < s->nelements; c++)
	{
		dump_element(c, &(s->elements[c]));
	}
}

void
dump_interchange(edi_interchange_t *i)
{
	size_t c;
	
	for(c = 0; c < i->nsegments; c++)
	{
		dump_segment(c, &(i->segments[c]));
	}
}

int
main(int argc, char **argv)
{
	char buf[2048];
	edi_parser_t *p;
	edi_interchange_t *i;
	int c;
	
	(void) argc;
	(void) argv;
	
	p = edi_parser_create(NULL);
	i = edi_parser_parse(p, msg1);
	
	edi_interchange_build(i, NULL, buf, sizeof(buf));
	
	fprintf(stderr, "Source:\n%s\n", msg1);
	fprintf(stderr, "Generated:\n%s\n", buf);
	c = strcmp(msg1, buf);
	if(c)
	{
		fprintf(stderr, "Source and generated versions differ\n");
		puts("FAIL");
	}
	else
	{
		puts("PASS");
	}
	edi_interchange_destroy(i);
		
	edi_parser_destroy(p);

	return c;
}
