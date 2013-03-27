<?php
$key = "d08b6ec8eec01d8a238d285840a7af1adb359c8f04b9297940456a9fc127ac76";

// Validation – make sure that we have the right information

if( ! isset($_POST['key']) ||  $_POST['key'] != $key) { 
	die ("Unknown key");
}

$msg  = "Welcome. You have successfully \n";
$msg .= "received a reply from the API.";

print "$msg";


?>
