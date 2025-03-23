This is an Electronic Data Interchange (EDI) parser, written in C.

There are several different flavours of EDI, each with their own spin on which
separator and escape characters are used, but the basics are universal. This
library allows you to configure the parser to the dialect of EDI you're dealing
with.

Note that EDI is not a descriptive message-passing format: unlike many XML-based
formats, it contains very little information about what the message is and how
it's supposed to be structured; you're expected to know that already if you're
sending or receiving the messages.

To build from a git clone, run:

```
$ autoreconf -fvi
$ ./configure --prefix=/opt/libedi
$ make check
$ make install
```

You will need to have `autoconf`, `automake` and `libtool` installed — they
should be available via your system package manager.

By default (if `clang` or `gcc` is detected), `libedi` will be built with
`-Werror -O0 -g`; this can be disabled by passing `--disable-debug` to
`configure`.

## References:

### EDI

* http://en.wikipedia.org/wiki/Electronic_Data_Interchange
* http://www.theedizone.com/edi_resources/edi_standards.html
	
### UN/EDIFACT

* http://en.wikipedia.org/wiki/EDIFACT
* http://www.unece.org/trade/untdid/texts/d422_d.htm
