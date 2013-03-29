#!/usr/bin/perl
use strict;

# Read in configuration file
my %config 	= do 'config.pl';
my $key 	= $config{'key'} or die ("Error in config.pl, can't read key. Quitting\n");
my $server 	= $config{'url'} or die ("Error in config.pl, can't read server URL. Quitting\n");

# sansrts.pl requires a single command-line parameter, the payload
die "Usage: sansrts.pl DATA\n\n" if (scalar(@ARGV) != 1);
my $payload = shift(@ARGV);
	
my $reply = `curl -s $server/API.php --data-urlencode key='$key' --data-urlencode payload='$payload'`;

print $reply;
