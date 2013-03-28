<?php
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Connect to database server
$db = mysqli_connect ($domain, $db_user, $db_pass, "sansgrid") or die ("Couldn't connect to server.");

// If a key wasn't provided, then exit out.
if ( ! isset($_POST['key']) )
	die ("Missing key"); 

$router_key = $_POST['key'];

// Check that the key provided is a known key
$query = "SELECT COUNT(*) AS count FROM router WHERE router_key='$router_key'";
$result = mysqli_query($db, $query) or die ("poop");
$row = mysqli_fetch_assoc($result);

if ( $row['count'] < 1) 
	die ("Unknown key"); 

if ( $row['count'] > 1) 
	die ("Duplicate key error. Contact system administrator"); 



$msg  = "Welcome. You have successfully \n";
$msg .= "received a reply from the API.";

print "$msg";


?>
