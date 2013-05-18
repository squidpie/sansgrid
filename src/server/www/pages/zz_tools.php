<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

/* ************************************************************************** */
//
function deleteSensorByID ($id_sensor) {
	
	$db = returnDatabaseConnection();

	// Delete all I/O associated with sensor
	$query = "DELETE FROM io WHERE id_sensor='$id_sensor'";
	mysqli_query($db, $query) 
		or die ("Can't execute query dsbi1\n");

	// Delete the sensor
	$query = "DELETE FROM sensor WHERE id_sensor='$id_sensor'";
	mysqli_query($db, $query) 
		or die ("Can't execute query dsbi2");
}
/* ************************************************************************** */


/* ************************************************************************** */
// Returns a random hex string that's $length characters long
function generateRandomHash ($length) {
	$tmp = "";
	for ($i = 0; $i < $length; ++$i) {
		$tmp .= sprintf ("%x", mt_rand(0,15));
	}


	return $tmp;
}
/* ************************************************************************** */


/* ************************************************************************** */
// Since we take hex numbers and treat them as strings in the database, we have
// to be ready for the fact that there may be leading zeroes in the number. 
//   e.g.:  0d13 = d13
function cleanZeroes ($str) {

	// Strip all leading zeroes
	$str = preg_replace ('/^0*/', '', $str); 

	// Now if string was nothing but 0's then it's now empty. Let's fix that.
	$str = $str == "" ? 0 : $str;

	return $str;
}
/* ************************************************************************** */


/* ************************************************************************** */
// The php internal XOR function doesn't work with large values.  So we have
// our own XOR function now.  This function takes in two hexadecimal numbers
// and returns a binary string of those two numbers XOR'd.
function sgXor ($num1, $num2) {
	
	// Convert to a binary string
	$num1 = base_convert ($num1, 16, 2);
	$num2 = base_convert ($num2, 16, 2);

	// Ensure that both strings have the proper length
	$num1 = zeroPad($num1);
	$num2 = zeroPad($num2);

	// Build the XOR'd response MSB to LSB
	$xord = "";
	for ($i = 0 ; $i < strlen($num1); ++$i) {

		$bit1 = substr ($num1, $i, 1);
		$bit2 = substr ($num2, $i, 1);

		$xord .= $bit1 == $bit2 ? 0 : 1;
	}

	return $xord;
} // END sgXor ($num1, $num2)
/* ************************************************************************** */


/* ************************************************************************** */
// Takes a binary string in and returns that same string padded with zeroes 
// to ensure that it is appropriate length as determined by $SG['skl']
function zeroPad ($bin_string) {
	global $SG;

	$byte_count = $SG['skl'] / 2;				// 2 characters per byte

	$max_binary_length = 8 * $byte_count;		// 8 bits per byte

	$diff = $max_binary_length - strlen($bin_string); // How many 0s do we need?

	$new = "";
	for ($i = 0; $i < $diff; ++$i) {
		$new .= "0";
	}

	$new .= $bin_string;
	return $new;
} // END zeroPad ($bin_string)
/* ************************************************************************** */


/* ************************************************************************** */
// Takes a string in and returns a count of how many 1s it contains
function countOnes ($str) {
	$num = 0;
	for ($i = 0 ; $i < strlen($str); ++$i) {
		$num += substr ($str, $i, 1);
	}
	return $num;
} // END countOnes ($str)
/* ************************************************************************** */


/* ************************************************************************** */
// Returns a connection to the MySQL database
function returnDatabaseConnection() {
	global $SG;
	$domain  = $SG['domain'];
	$db_user = $SG['db_user'];
	$db_pass = $SG['db_pass'];

	$db = @mysqli_connect("$domain", "$db_user", "$db_pass", "sansgrid") 
			or die ("Couldn't connect to database.<br>$query");
	return $db;
}
/* ************************************************************************** */


/* ************************************************************************** */
//
function appendToPayload ($current_payload, $key, $value) {
	global $SG;
	$current_payload .= $key . $SG['kv_del'] . $value . $SG['ff_del'];

	return $current_payload;
}
/* ************************************************************************** */


/* ************************************************************************** */
// Get's the current refresh rate for auto-refresh'able pages (index, pipeline, /i
// logs).
function returnRefresh () {

	$db = returnDatabaseConnection();

	$query = "SELECT refresh_rate FROM server";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query rr1.");
	$row = mysqli_fetch_assoc($result);
	$refresh_rate = $row['refresh_rate'];

	if ($refresh_rate == 0)
		return "";
	
	return "<meta http-equiv=\"refresh\" content=\"$refresh_rate\">";

}
/* ************************************************************************** */




/* ************************************************************************** */
// Get's the current refresh rate for auto-refresh'able pages (index, pipeline, /i
// logs).
function takeSensorOffline ($rdid) {

	$db = returnDatabaseConnection();

	// Let's first get the id_sensor for the log
	$query = "SELECT id_sensor FROM sensor WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query tso2.");
	$row = mysqli_fetch_assoc($result);
	$id_sensor = $row['id_sensor'];

	// If we don't see this rdid, then we're done I guess.
	if ( $id_sensor == " ")
		return;

	// Log it
	$msg  = "Sensor ($id_sensor) now offline. ";
	addtolog($msg);

	// Now we take the sensor offline and remove the the rdid
	$query  = "UPDATE sensor SET status='offline', rdid='' ";
	$query .= "WHERE id_sensor='$id_sensor'";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query tso3.");


} // End takeSensorOffline()

/* ************************************************************************** */


/* ************************************************************************** */
//
function xmitToRouter ($outbound_payload, $url="") {

	print "This doesn't belong here (zz_tools.php) but: $outbound_payload\n";
	#return;

	# DELETE THIS! THIS IS JUST FOR DEBUGGING!!!!
	if ($url == "10.42.0.1")
		$url = "10.42.0.40";

	#$url="10.42.0.40/API-router.php";
	$url="$url/API-router.php";

	// Make it safe to transmit via http

	// Package it as HTTP data
	$data = "payload=$outbound_payload";

	//Instatiate a curl handle
	$ch = curl_init();

	//Set the router url, number of POST vars, and finally the POST data
	curl_setopt($ch	,CURLOPT_URL, 			$url);
	curl_setopt($ch	,CURLOPT_POST, 			1);		// Passing 1 variable
	curl_setopt($ch	,CURLOPT_POSTFIELDS, 	$data);

	//Do it!
	$junk = curl_exec($ch);		// Ignoring anything returned

	//Did it...
	curl_close($ch);


}
/* ************************************************************************** */
?>
