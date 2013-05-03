#!/usr/bin/perl

# If we were passed a sleep time we'll use that, otherwise: sleep = 3 seconds
$sleep = ( scalar(@ARGV) > 0 ) ? shift(@ARGV) : 3;

while ( $file = glob("*payload") ) {

	print "Next: $file\n";
	
	# Assumes that sansrts.pl is in the $PATH
	`sansrts.pl "\$(cat $file)"`;

	for ( $i = 0; $i < $sleep; ++$i) {
		print ".";
		sleep 1;
	}
	print "\n";

}
