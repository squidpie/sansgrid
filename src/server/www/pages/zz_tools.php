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
	$current_payload .= $key . $SG['kv_del'] . $value . $SG['pp_del'];

	return $current_payload;
}
/* ************************************************************************** */


/* ************************************************************************** */
//
function xmitToRouter ($outbound_payload) {

	print $outbound_payload;
}
/* ************************************************************************** */
?>
