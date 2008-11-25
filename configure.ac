dnl @(#) $Id$

dnl Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Mo McRoberts.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl 3. The names of the author(s) of this software may not be used to endorse
dnl    or promote products derived from this software without specific prior
dnl    written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
dnl INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
dnl AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
dnl AUTHORS OF THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
dnl SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
dnl TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
dnl PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
dnl LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
dnl NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
dnl SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AC_INIT(libedi, 1.0.2)
AC_CONFIG_SRCDIR([libedi/parse.c])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

CPPFLAGS="$CPPFLAGS -W -Wall -D_REENTRANT"
export CPPFLAGS

AC_PROG_CC
AC_PROG_LIBTOOL

AC_ARG_ENABLE(pthread, [AS_HELP_STRING([--enable-pthread],[Use POSIX threads (default=auto)])],[use_pthread=$enableval],[use_pthread=auto])

AC_MSG_CHECKING([whether to use POSIX threads])
AC_MSG_RESULT([${use_pthread}])
if test x"${use_pthread}" = x"auto" || test x"${use_pthread}" = x"yes" ; then
	pthread_present=yes
	AC_CHECK_HEADER([pthread.h],[AC_DEFINE_UNQUOTED([HAVE_PTHREAD_H],1,[Define to 1 if you have the <pthread.h> header file.])],pthread_present=no)
	if test x"${pthread_present}" = x"yes" ; then
		AC_CHECK_LIB(pthread,pthread_create)
		AC_CHECK_FUNC(pthread_create,,pthread_present=no)
	fi
	if test x"${pthread_present}" = x"no" && test x"${use_pthread}" = x"yes"; then
		AC_MSG_ERROR([POSIX threads support was requested but it cannot be located])
	fi
	use_pthread=${pthread_present}
fi

if test x"${use_pthread}" = x"yes" ; then
	AC_DEFINE_UNQUOTED([LIBEDI_USE_PTHREAD], 1, [Define to use POSIX threads for locking])
else
	use_pthread=no
fi

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile
include/Makefile
libedi/Makefile
tests/Makefile
])

AC_OUTPUT
