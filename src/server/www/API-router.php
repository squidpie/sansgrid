<?php
$server_key = "";

/* ************************************************************************** */
/* START OF MAIN ************************************************************ */

$kv_del = "α";						// Key-value delimiter
$ff_del = "β";						// Parameter-to-parameter delimiter

// The IP address of whomever's connecting to the API?
#$router_ip = $_SERVER['REMOTE_ADDR'];

// If a key wasn't provided, then exit out.
#if ( ! isset($_POST['key']) ) {
#	die;
#}

#$router_key = $_POST['key'];

// If a payload wasn't provided, then exit out.
#if ( ! isset($_POST['payload']) ) {
#	die;
#}


// If we've gotten here, the server is believed to be authentic and we accept
// the data as legit. 

// Decode from HTTP encoding...
$payload = urldecode($_POST['payload']);

// ...then encode (escaping, really) as a safe shell parameter
//$payload = escapeshellarg($payload);

// Now give it to the router daemon. 
$caller = "/usr/local/bin/sansgridrouter --packet=";
$umm = exec("$caller $payload");

#print "\nAPI-router received the data. Called the following:\n$caller $payload\n$umm\n";

