/* test-3: X12 source parsing, should produce broken output by default because
 * the interchange header won't match the actual output.
 */

#include <stdio.h>
#include <string.h>

#include "libedi.h"

/* Input */
const char *msgin = 
	"ISA:00:          :00:          :01:1515151515     :01:5151515151     :041201:1217:U:00304:000032123:0:P:*~"
	"GS:CT:9988776655:1122334455:041201:1217:128:X:003040~"
	"ST:831:00128001~"
	"BGN:00:88200001:041201~"
	"N9:BT:88200001~"
	"TRN:1:88200001~"
	"AMT:2:100000.00~"
	"AMT:2:200000.00~"
	"AMT:2:300000.00~"
	"AMT:2:400000.00~"
	"QTY:41:1~"
	"QTY:41:2~"
	"QTY:41:3~"
	"QTY:41:4~"
	"SE:7:00128001~"
	"GE:1:128~"
	"IEA:1:000032123~";

/* Output */
const char *msgout = 
	"ISA+00+          +00+          +01+1515151515     +01+5151515151     +041201+1217+U+00304+000032123+0+P+'"
	"GS+CT+9988776655+1122334455+041201+1217+128+X+003040'"
	"ST+831+00128001'"
	"BGN+00+88200001+041201'"
	"N9+BT+88200001'"
	"TRN+1+88200001'"
	"AMT+2+100000.00'"
	"AMT+2+200000.00'"
	"AMT+2+300000.00'"
	"AMT+2+400000.00'"
	"QTY+41+1'"
	"QTY+41+2'"
	"QTY+41+3'"
	"QTY+41+4'"
	"SE+7+00128001'"
	"GE+1+128'"
	"IEA+1+000032123'";

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
	i = edi_parser_parse(p, msgin);
	
	edi_interchange_build(i, NULL, buf, sizeof(buf));
	
	fprintf(stderr, "Source:\n%s\n", msgin);
	fprintf(stderr, "Expected:\n%s\n", msgout);
	fprintf(stderr, "Generated:\n%s\n", buf);
	c = strcmp(msgout, buf);
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
