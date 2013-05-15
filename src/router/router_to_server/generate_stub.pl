#!/usr/bin/perl
use strict; 
use Switch;				# Provides switch-case block
use warnings;

# ############################################################################## # VARIABLE DECLARATIONS
my @a;
my $user_request;

my $nf_del = "α";		# Name-Field delimiter (intra-parameter)
my $ff_del = "β";		# Parameter-Parameter delimiter (inter-parameter)

# ############################################################################## 
# START OF MAIN CODE

#&clear;
print "\nSansGrid Project\n";
print "Test generator for sansrts.pl\n";
print "-----------------------------\n\n";

print "Choose a process to emulate:\n";
printf ("%4d  -  %s\n", 1, "Eyeball");
printf ("%4d  -  %s\n", 2, "Mock");
printf ("%4d  -  %s\n", 3, "Peacock");
printf ("%4d  -  %s\n", 4, "Squawk");
#printf ("%4d  -  %s\n", 5, "Squawk");
printf ("%4d  -  %s\n", 6, "Sensor status updates (0xFD)");

print "\n";
$user_request = &verifyInput( (1,2,3,4) );

switch ($user_request) {
	case 1 { &generate_eyeball();}
	case 2 { &generate_mock();}
	case 3 { &generate_peacock();}
	case 4 { &generate_squawk();}
	case 6 { &generate_status_update();}
}

# END OF MAIN CODE
# ############################################################################## 


# ############################################################################## 
# generate_eyeball()
#
#	Generate eyeball payload
#
sub generate_eyeball {

	my $input;
	my $payload = $ff_del . "dt" . $nf_del . "00" . $ff_del;

	#rdid
	$payload .= "rdid" . $nf_del . int(rand(64000)) . $ff_del;


	# Data type
	print "Data type: 0x01\n";

	# manid
	$payload .= "manid" . $nf_del . &getField("Manufactuer's ID # (4 bytes)") . $ff_del;

	# modnum
	$payload .= "modnum" . $nf_del . &getField("Model # (4 bytes)") . $ff_del;

	# sn
	$payload .= "sn" . $nf_del . &getField("Serial Number (8 bytes)") . $ff_del;

	# profile
	print "'profile' left blank\n";
	$payload .= "profile" . $nf_del . "" . $ff_del;

	# mode
	$payload .= "mode" . $nf_del . &getField("Mode (4 bits)") . $ff_del;

	&savePayload($payload);

} # End generate_eyeball


# ############################################################################## 
# generate_mock()
#
#	Generate Mock payload
#
sub generate_mock {

	my $input;
	my $payload;

	#rdid
	$payload .= $ff_del . "rdid" . $nf_del . &getField("rdid") . $ff_del;


	print "Would you like to include a sensor authentication key?";
	print "\n\n";
	$user_request = &verifyInput( ("y","n") );

	if ($user_request eq "y") {

		# Data type
		$payload .= "dt" . $nf_del . "07" . $ff_del;

		$user_request = &getField("Sensor Public Key (64 bytes or type 'R' to generate a random key)\n");

		if ($user_request eq "R") {
			$user_request =  "";
			for (my $i = 0; $i < 128; ++$i) {
				$user_request .= sprintf ("%x", int(rand(15)));
			}
		}

		$payload .= "senspubkey" . $nf_del . $user_request . $ff_del;

	} else {
		$payload .= "dt" . $nf_del . "08" . $ff_del;
		$payload .= "senspubkey" . $nf_del . $ff_del;
	}


	&savePayload($payload);

} # End generate_mock 


# ############################################################################## 
# generate_peacock()
#
#	Generate peacock payload
#
sub generate_peacock {

	my $input;
	my $payload;

	#dt
	$payload .= $ff_del . "dt" . $nf_del . "0c" . $ff_del;

	#rdid
	$payload .= "rdid" . $nf_del . &getField("rdid") . $ff_del;


	print "\nSIGNAL A\n";
	$payload .= "sida" . $nf_del . &getField("ID #") . $ff_del;

	print "Classification ( 0 = digital, 1 = analog, 2 = text )";
	$user_request = &verifyInput( (0, 1, 2) );
	$payload .= "classa" . $nf_del . $user_request . $ff_del;

	print "Direction ( 0 = output from sensor, 1 = input )";
	$user_request = &verifyInput( (0, 1) );
	$payload .= "dira" . $nf_del . $user_request . $ff_del;

	$payload .= "labela" . $nf_del . &getField("Label") . $ff_del;

	$payload .= "unitsa" . $nf_del . &getField("Units") . $ff_del;

	print "Would you like to include another I/O ('B')?";
	$user_request = &verifyInput( ("y", "n") );

	# If there's a 'B' I/O
	if ($user_request eq "y") {

		print "\nSIGNAL B\n";
		$payload .= "sidb" . $nf_del . &getField("ID #") . $ff_del;

		print "Classification ( 0 = digital, 1 = analog, 2 = text )";
		$user_request = &verifyInput( (0, 1, 2) );
		$payload .= "classb" . $nf_del . $user_request . $ff_del;

		print "Direction ( 0 = output from sensor, 1 = input )";
		$user_request = &verifyInput( (0, 1) );
		$payload .= "dirb" . $nf_del . $user_request . $ff_del;

		$payload .= "labelb" . $nf_del . &getField("Label") . $ff_del;

		$payload .= "unitsb" . $nf_del . &getField("Units") . $ff_del;

		print "Are there any more Peacocks coming?";
		$user_request = &verifyInput( ("y", "n") );

		if ($user_request eq "y") {
			$payload .= "additional" . $nf_del . "1" . $ff_del;
		} else {
			$payload .= "additional" . $nf_del . "0" . $ff_del;
		}

	# If there isn't a 'B' I/O
	} else {

		$payload .= "sidb" . $nf_del . "fd" . $ff_del;

		$payload .= "classb" . $nf_del . $ff_del;

		$payload .= "dirb" . $nf_del . $ff_del;

		$payload .= "labelb" . $nf_del . $ff_del;

		$payload .= "unitsb" . $nf_del . $ff_del;

		$payload .= "additional" . $nf_del . "0" . $ff_del;

	}


	&savePayload($payload);

} # End generate_peacock


# ############################################################################## 
# generate_squawk()
#
#	Generate peacock payload
#
sub generate_squawk {

	my $input;
	my $payload;

	#rdid
	$payload .= $ff_del . "rdid" . $nf_del . &getField("rdid") . $ff_del;


	print "Choose a squawk type:\n";
	print "  1) Sensor response to server challenge, no sensor challenge.\n";
	print "  2) Sensor response to server challenge, sensor challenge coming.\n";
	print "  3) Sensor challenge to server.\n";
	print "  4) Sensor accepts server's response.\n";
	$user_request = &verifyInput( (1, 2, 3, 4) );

	switch ($user_request) {

		#1) Sensor response to server challenge, no sensor challenge.
		case 1 { 
			$payload .= "dt" . $nf_del . "15" . $ff_del;
			$payload .= "data" . $nf_del
						. &getField("Challenge response") . $ff_del;

		}

		# 2) Sensor response to server challenge, sensor challenge coming.
		case 2 { 
			$payload .= "dt" . $nf_del . "16" . $ff_del;
			$payload .= "data" . $nf_del
						. &getField("Challenge response") . $ff_del;

		}

		# 3) Sensor challenge to server.
		case 3 { 
			$payload .= "dt" . $nf_del . "17" . $ff_del;

			$user_request = &getField("Challenge (type 'R' to generate a random challenge)\n");

			if ($user_request eq "R") {

				$user_request = &getField("OK, how many characters (not bytes!)?\n");
				my $count = $user_request;

				$user_request =  "";
				for (my $i = 0; $i < $count; ++$i) {
					$user_request .= sprintf ("%x", int(rand(15)));
				}

			}

			$payload .= "data" . $nf_del . $user_request . $ff_del;

		}

		# 4) Sensor accepts server's response.
		case 4 { 
			$payload .= "dt" . $nf_del . "1d" . $ff_del;
			$payload .= "data" . $nf_del . $ff_del;

		}
	}

	&savePayload($payload);

} # End generate_squawk


# ############################################################################## 
# generate_status_update()
#
#	Generate peacock payload
#
sub generate_status_update {

	my $input;
	my $payload;

	#rdid
	$payload .= $ff_del . "rdid" . $nf_del . &getField("rdid") . $ff_del;
	$payload .= "dt" . $nf_del . "fd" . $ff_del;


	print "Status update:\n";
	print "  1) online\n";
	print "  2) stale\n";
	print "  3) offline\n";
	$user_request = &verifyInput( (1, 2, 3) );

	switch ($user_request) {

		case 1 { 
			$payload .= "data" . $nf_del . "online" . $ff_del;
		}

		case 2 { 
			$payload .= "data" . $nf_del . "stale" . $ff_del;
		}
		
		case 3 { 
			$payload .= "data" . $nf_del . "offline" . $ff_del;
		}
	}

	# THIS IS WHERE I LEFT OFF!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	&savePayload($payload);

} # End generate_status_update


# ############################################################################## 
# getField($user_input)
sub getField {
	my $user_prompt = shift;
	my $input;

	print "$user_prompt? ";
	$input = <STDIN>;
	$input =~ s/^\s*//;	
	$input =~ s/\s*$//;	

	return $input;
}


# ############################################################################## 
# verifyInput(@valid_inputs) 
#
#	Takes in an array of valid inputs and continues to prompt user for a reply
# 	until their reply matches (begins with) a valid input
#
sub verifyInput() {
	my @valid_inputs = @_;			# Passed in acceptable values
	my $good_input = 0;				# Set to 1 if user input is in @valid_inputs
	my $user_input;

	# Until we get a good input
	until ($good_input == 1) {

		print "(";
		# Print options for user
		foreach (@valid_inputs) {
			print "$_,";
		}
		print " or Q)? ";

		# Get input from user
		$user_input = <STDIN>;
		chomp($user_input);

		# Try and set $good_input
		foreach (@valid_inputs) {
			$good_input = ($user_input =~ m/^$_/i) ? 1 : $good_input;	
		}

		die ("\nQuitting by user request.\n\n") if ($user_input =~ m/^q/i);
	}

	return $user_input;
}



# ############################################################################## 
# savePayload($payload)
#
sub savePayload {
	my $payload = shift;
	my $input;
	
	#&clear;
	print "\n\n";
	print "Would you like to save to a file (f) or print to screen (s)\n";
	$input = &verifyInput( ("f", "s") );


	# Save to file
	if ($input =~ m/^f/i) {
		print "\n\nFilename? ";
		$input = <STDIN>;
		$input =~ s/^\s*//;
		$input =~ s/\s*$//;

		open (MYFILE, ">$input") or die ("I'm sorry I can't create the file '$input'. Quitting\n");
		print MYFILE $payload;
		close MYFILE;
		
		print "\n\nPayload saved to '$input'\n\n";

	# Print to screen
	} elsif ($input =~ m/^s/i) {
		print "\n\n$payload\n\n";
	}

}

# ############################################################################## 
sub clear {
	print "\n" x 100;
}
