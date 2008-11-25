#! /bin/sh

exec 2>tests.log
r=0

runtest()
{
	echo "Starting test: $1" >&2
	out=`$1`
	echo "Result: $out" >&2
	case "$out" in
		PASS|XFAIL)
			echo "$1: $out"
			;;
		*)
			echo "*** $1: $out ***"
			r=1
	esac
}

echo "Test run started at `date`" >&2

runtest ./test-1
runtest ./test-2
runtest ./test-3

echo "Test run completed at `date`" >&2

exit $r

