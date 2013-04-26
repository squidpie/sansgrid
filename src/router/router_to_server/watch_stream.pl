#!/usr/bin/perl

`touch ./stream.text` unless ( -f "./stream.text");

while (1) {
	print "\n" x 3;
	print `tail -n 1 ./stream.text`;
	sleep 1;
}
