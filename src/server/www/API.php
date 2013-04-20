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
	$msg = "[$router_ip] - Duplicate key error! Terminated connection.";
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
switch ( $data["dt"] ) {
	case 0:
		processEyeball($router_ip, $data, $db);
		break;
	default: 
		die ("Error: Unknown data type: '${data["dt"]}'\n");
		break; 	# Technically not needed since we just died.
}

/* END OF MAIN ************************************************************** */
/* ************************************************************************** */


/* ************************************************************************** */
function processEyeball ($router_ip, $payload, $db) {
	global $SG;

	// Data from API
	$rdid 		= $payload["rdid"];
	$modnum 	= $payload["modnum"];
	$manid 		= $payload["manid"];
	$sn 		= $payload["sn"];
	$mode 		= $payload["mode"];

	// First thing to do is ensure that this rdid isn't currently in the pipeline
	deleteFromPipelineByRdid ($rdid, $db);

	// All replies should include the originating rdid
	$reply = appendToPayload("", "${SG['ff_del']}rdid", $rdid);
	
	// Do we recognize this sensor?
	$query  = "SELECT COUNT(*) as count FROM sensor ";
	$query .= "WHERE modnum='$modnum' AND manid='$manid' ";
	$query .= "AND  sn='$sn'";
	$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query EB1.");

	$row = mysqli_fetch_assoc($result);
	$count = $row['count'];

	// Now we generate the appropriate Peck, start by getting the server key.
	$query = "SELECT server_id FROM server";
	$result = mysqli_query ($db, $query) or die ("Error: Couldn't execute query EB2.");
	$row = mysqli_fetch_assoc($result);
	$server_id = $row['server_id'];

	// If sensor is recognized
	if ($count == 1)  {
		
		$reply = appendToPayload($reply, "dt", 			 "1");
		$reply = appendToPayload($reply, "ipaddress", 	 "");
		$reply = appendToPayload($reply, "serverid", 	 $row['server_id']);
		$reply = appendToPayload($reply, "recognition",	 "0");
		$reply = appendToPayload($reply, "manid", 		 $manid);
		$reply = appendToPayload($reply, "modnum", 		 $modnum);
		$reply = appendToPayload($reply, "sn", 			 $sn);
		
		xmitToRouter ($reply);


		// SQUAWKING BELONGS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	

	// .. else sensor is not recognized
	} else {
		// If it's a new sensor, we should mate.  Is the sensor ready to mate?

		//  if sensor not ready to mate...
		if ($mode == 0) {

			$reply = appendToPayload($reply, "dt", 				"1");
			$reply = appendToPayload($reply, "ipaddress", 		"");
			$reply = appendToPayload($reply, "serverid", 		$server_id);
			$reply = appendToPayload($reply, "recognition", 	"3");
			$reply = appendToPayload($reply, "manid", 			$manid);
			$reply = appendToPayload($reply, "modnum", 			$modnum);
			$reply = appendToPayload($reply, "sn", 				$sn);

			// Log it
			$msg  = "[$router_ip] - Eyeballing - New sensor entered network. ";
			$msg .= "Sensor not ready to mate. ";
			$msg .= "Manufacturer's ID = $manid, ";
			$msg .= "model number = $modnum, ";
			$msg .= "serial number = $sn";
			addToLog($msg);

			print "New sensor. Sensor not ready to mate.\n";
			xmitToRouter($reply);

		// ...else sensor is ready to mate
		} else {

			// Is server set to allow automatic mating?
			$query = "SELECT verify_mating FROM server";
			$result = mysqli_query($db, $query) or die ("Couldn't execute query EB3.");
			$row = mysqli_fetch_assoc($result);
			$verify_mating = $row['verify_mating'];

			// If automatic mating is allowed...
			if ($verify_mating == 0) {

				$reply = appendToPayload($reply, "dt", 				"1");
				$reply = appendToPayload($reply, "ipaddress", 		"");
				$reply = appendToPayload($reply, "serverid", 		$server_id);
				$reply = appendToPayload($reply, "recognition", 	"1");
				$reply = appendToPayload($reply, "manid", 			$manid);
				$reply = appendToPayload($reply, "modnum", 			$modnum);
				$reply = appendToPayload($reply, "sn", 				$sn);

				xmitToRouter($reply);

				// Sleep for 250 ms after Peck before sending Sing. 
				usleep(250000);		

				// Now we build our Sing payload.
				// All replies should include the originating rdid
				$reply = appendToPayload("", "${SG['ff_del']}rdid", $rdid);

				// Do we have a server key?
				$query = "SELECT server_key FROM server";
				$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query si1.");
				$row = mysqli_fetch_assoc($result);
				$server_key = $row['server_key'];

				//                             ↘No key   ↘Yes key
				$dt = $server_key == "" ? $dt = 3 : $dt = 2;

				// Finishing Sing payload
				$reply = appendToPayload($reply, "dt", 			$dt);
				$reply = appendToPayload($reply, "serverkey", 	$server_key);

				xmitToRouter($reply);

				// Update pipeline
				updatePipeline ($rdid, "", 2);



			// ...else automatic mating is not allowed
			} else {
				$reply = appendToPayload($reply, "dt", 			"1");
				$reply = appendToPayload($reply, "ipaddress", 	"");
				$reply = appendToPayload($reply, "serverid", 	$server_id);
				$reply = appendToPayload($reply, "recognition",	"4NEEDAPPROVAL");
				$reply = appendToPayload($reply, "manid", 		$manid);
				$reply = appendToPayload($reply, "modnum", 		$modnum);
				$reply = appendToPayload($reply, "sn", 			$sn);

				print "New sensor. Sensor ready to mate. Server requires authentication.\n";
				print "WARNING!!! We need a new reply for this\n";
				print "WARNING!!! this still needs to be added to the pipeline\n";
				xmitToRouter($reply);

				// NEED TO ADD THIS TO PIPELINE AS A SENSOR WAITING TO MATE
			}
			
		}
	}
}


?>
