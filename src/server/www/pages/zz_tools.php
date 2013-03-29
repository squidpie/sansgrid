<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

/* ************************************************************************** */
function generateRandomHash ($length) {
	$tmp = "";
	for ($i = 0; $i < $length; ++$i) {
		$tmp .= sprintf ("%x", mt_rand(0,15));
	}


	return $tmp;
}
/* ************************************************************************** */


/* ************************************************************************** */
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
?>
