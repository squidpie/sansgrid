<?php
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

/* ************************************************************************** */
/* START OF MAIN ************************************************************ */

// The IP address of whomever's connecting to the API?
$router_ip = $_SERVER['REMOTE_ADDR'];

// Connect to database server
$db = returnDatabaseConnection();

// If a key wasn't provided, then exit out.
if ( ! isset($_POST['key']) ) {
	$msg = "[$router_ip] - Missing key, terminated connection.";
	addToLog($msg);
	die ("Missing key"); 
}

$router_key = $_POST['key'];

// If a payload wasn't provided, then exit out.
if ( ! isset($_POST['payload']) ) {
	$msg = "[$router_ip] - Missing payload. Terminated connection.";
	addToLog($msg);
	die ("Missing payload"); 

// DEBUGGING 
} else {
	if ( $SG['debug'] == true ) {
		$msg = "DEBUG:<br>" . debugSplitting($_POST['payload']);
		addToLog($msg);
	}
}

$ff_del = $SG['ff_del'];
$raw_payload = urldecode($_POST['payload']);
$raw_payload = preg_replace("/^$ff_del/", "",  $raw_payload);
$raw_payload = preg_replace("/$ff_del\$/", "",  $raw_payload);

// Check that the key provided is a known key
$query = "SELECT COUNT(*) AS count FROM router WHERE router_key='$router_key'";
$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query 1.");
$row = mysqli_fetch_assoc($result);

// Unknown key...
if ( $row['count'] < 1)  {
	$msg = "[$router_ip] - Unknown key. Terminated connection.";
	addToLog($msg);
	die ("Unknown key"); 
}

// Duplicate key...
if ( $row['count'] > 1) {
	$msg = "[$router_ip] - Duplicate router key error! Terminated connection.";
	addToLog($msg);
	die ("Duplicate key error. Contact system administrator"); 
}


// If we've gotten here, the router is believed to be authentic. Now we
// parse out the data and figure out what we're doing.  Let's start by
// splitting the payload into key/value pairs
$kvps = explode($SG['ff_del'], $raw_payload) or die ("Error: No KV pairs found in payload.");

// We should now have a minimum of two fields ('dt' and some data). 
if ( count($kvps) < 2) {
	$msg = "[$router_ip] - Poorly formatted payload.";
	addToLog($msg);
	die ("Error: Poorly formatted payload.");
}


// Now we go through the key/value pairs and push them into an associative array
foreach ($kvps as $kvp) {
	list($key, $value) = explode($SG['kv_del'], $kvp);
	$data["$key"] = $value;
}

if (! array_key_exists("dt", $data) ) {
	$msg = "[$router_ip] - No data type found in payload. Terminated connection.";
	addToLog($msg);
	die ("Error: no data type found in payload.");
}

// We're done parsing out the data.  Now we look at our data type and run
// the appropriate function for that data type. 
switch ( hexdec($data["dt"]) ) {
	case 235:	// 234 = 0xeb		
		processEyeball($router_ip, $data, $db);
		break;
	case 7:
		processMock($router_ip, $data, $db);
		break;
	case 8:
		processMock($router_ip, $data, $db);
		break;
	case 12: 	// 11 = 0x0C
		processPeacock($router_ip, $data, $db);
		break;
	case 21: 	// 21 = 0x15
		// Sensor's response to server's initial squawk  (No sensor challenge)
		processSquawkSensorReply($router_ip, $data, $db);
		break;
	case 22: 	// 22 = 0x16
		// Sensor's response to server's initial squawk (Sensor challenge coming)
		processSquawkSensorReply($router_ip, $data, $db);
		break;
	case 23: 	// 23 = 0x17
		// Sensor sending its own challenge
		processSquawkSensorChallenge($router_ip, $data, $db);
		break;
	case 29: 	// 29 = 0x1d
		// Sensor has accepts the server's response to its challenge
		processSquawkAcceptsChallenge($router_ip, $data, $db);
		break;
	case 33: 	// 33 = 0x21
		// Chirp data from sensor
		processChirpData($router_ip, $data, $db);
		break;
	case 37: 	// 37 = 0x25
		// If router sends a 0x25 we drop the device and, wierdly enough,
		// send a 0x25 back out.
		dropSensorFromNetwork($router_ip, $data, $db);
		break;
	case 38: 	// 38 = 0x26
		// If sensor sends a 0x26 we drop the device and, wierdly enough,
		// send a 0x25 back out.
		dropSensorFromNetwork($router_ip, $data, $db);
		break;
	case 39: 	// 39 = 0x27
		// If sensor sends a 0x27 we drop the device and completely forget
		// that it ever existed. 
		dropAndForgetSensor($router_ip, $data, $db);
		break;
	case 253: 	// 253 = 0xfd
		// Status updates from the router
		updateSensorStatus($router_ip, $data, $db);
		break;
	default: 
		die ("Error: Unknown data type: ' " . hexdec($data["dt"]) . " '\n");
		break; 	# Technically not needed since we just died.
}

/* END OF MAIN ************************************************************** */
/* ************************************************************************** */
