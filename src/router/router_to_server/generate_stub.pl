#!/usr/bin/perl
use strict; 
use Switch;				# Provides switch-case block
use warnings;

# ############################################################################## # VARIABLE DECLARATIONS
my @a;
my $user_request;

my $nf_del = "α";		# Name-Field delimiter (intra-parameter)
my $pp_del = "β";		# Parameter-Parameter delimiter (inter-parameter)

# ############################################################################## 
# START OF MAIN CODE

&clear;
print "\nSansGrid Project\n";
print "Test generator for sansrts.pl\n";
print "-----------------------------\n\n";

print "Choose a process to emulate:\n";
printf ("%4d  -  %s", 1, "Eyeballing");

print "\n\n";
$user_request = &verifyInput( (1) );

switch ($user_request) {
	case 1 { &generate_eyeball();}
}

# END OF MAIN CODE
# ############################################################################## 


# ############################################################################## 
# generate_eyeball()
#
#	Generate eyeball payload
#
sub generate_eyeball{

	my $input;
	my $payload = "dt" . $nf_del . "01" . $pp_del;

	# Data type
	print "Data type: 0x01\n";

	# manid
	$payload .= "manid" . $nf_del . &getField("Manufactuer's ID # (4 bytes)") . $pp_del;

	# modnum
	$payload .= "modnum" . $nf_del . &getField("Model # (4 bytes)") . $pp_del;

	# sn
	$payload .= "sn" . $nf_del . &getField("Serial Number (8 bytes)") . $pp_del;

	# profile
	print "'profile' left blank\n";
	$payload .= "profile" . $nf_del . "" . $pp_del;

	# mode
	$payload .= "mode" . $nf_del . &getField("Mode (4 bits)") . $pp_del;


	# Now we just need to remove that last $pp_del
	$payload =~ s/$pp_dell$//;

	&savePayload($payload);

}


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
	
	&clear;
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
