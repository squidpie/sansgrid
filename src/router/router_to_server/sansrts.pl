#!/usr/bin/perl
use strict;

my $config_path = '/home/kane/rpi_mount/sansgrid/src/router/router_to_server';

# Read in configuration file
my %config 	= do "$config_path/config.pl";

my $key 	= $config{'key'} or die ("Error in config.pl, can't read key. Quitting\n");
my $server 	= $config{'url'} or die ("Error in config.pl, can't read server URL. Quitting\n");

# sansrts.pl requires a single command-line parameter, the payload
die "Usage: sansrts.pl DATA\n\n" if (scalar(@ARGV) != 1);
my $payload = shift(@ARGV);

#`echo curl -s $server/API.php --data-urlencode key='$key' --data-urlencode payload='$payload' > ./last_data_sent.text`;
	
my $reply = `curl -s $server/API.php --data-urlencode key='$key' --data-urlencode payload='$payload'`;

print $reply;
