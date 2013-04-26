<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

/* ************************************************************************** */
// Returns a random hex stringth
function generateRandomHash ($length) {
	$tmp = "";
	for ($i = 0; $i < $length; ++$i) {
		$tmp .= sprintf ("%x", mt_rand(0,15));
	}


	return $tmp;
}
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
//
function xmitToRouter ($outbound_payload, $url="") {

	print "This doesn't belong here (zz_tools.php) but: $outbound_payload\n";
	#return;

	$url="10.42.0.40/API-router.php";

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
