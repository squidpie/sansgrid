<?php
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Connect to database server
$db = returnDatabaseConnection();

// If a key wasn't provided, then exit out.
if ( ! isset($_POST['key']) )
	die ("Missing key"); 

$router_key = $_POST['key'];

// If a payload wasn't provided, then exit out.
if ( ! isset($_POST['payload']) )
	die ("Missing payload"); 

$raw_payload = urldecode($_POST['payload']);

// Check that the key provided is a known key
$query = "SELECT COUNT(*) AS count FROM router WHERE router_key='$router_key'";
$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query 1.");
$row = mysqli_fetch_assoc($result);

// Unknown key...
if ( $row['count'] < 1) 
	die ("Unknown key"); 

// Duplicate key...
if ( $row['count'] > 1) 
	die ("Duplicate key error. Contact system administrator"); 


// If we've gotten here, the router is believed to be authentic. Now we
// parse out the data and figure out what we're doing.  Let's start by
// splitting the payload into key/value pairs
$kvps = explode($SG['pp_del'], $raw_payload) or die ("Error: No KV pairs found in payload.");

// Now we go through the key/value pairs and push them into an associative array
foreach ($kvps as $kvp) {
	list($key, $value) = explode($SG['kv_del'], $kvp);
	$data["$key"] = $value;
}

// We're done parsing out the data.  Now we look at our data type and run
// the appropriate function for that data type. 
switch ( $data["dt"] ) {
	case 0:
		processEyeball($data, $db);
		break;
	default: 
		die ("Error: Unknown data type: '${data["dt"]}'\n");
		break; 	# Technically not needed since we just died.
}

/* ************************************************************************** */
function processEyeball ($payload, $db) {
	global $SG;

	$modnum = $payload["modnum"];
	$manid 	= $payload["manid"];
	$sn 	= $payload["sn"];
	
	// Do we recognize this sensor?
	$query  = "SELECT COUNT(*) as count FROM sensor ";
	$query .= "WHERE modnum='$modnum' AND manid='$manid' ";
	$query .= "AND  sn='$sn'";
	$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query EB1.");

	$row = mysqli_fetch_assoc($result);
	$count = $row['count'];

	print "There are $count sensors\n";
	// Now we generate the appropriate Peck

	// If sensor is recognized
	if ($count == 1)  {

		$query = "SELECT server_key FROM server WHERE id_server = 1";
		$result = mysqli_query ($db, $query) or die ("Error: Couldn't execute query EB2.");
		$row = mysqli_fetch_assoc($result);
		$server_key = $row['server_key'];
		
		$reply  = "dt" 			. $SG['kv_del'] . "1"					. $SG['pp_del'];
		$reply .= "ipaddress" 	. $SG['kv_del'] . ""					. $SG['pp_del'];
		$reply .= "serverid" 	. $SG['kv_del'] . $row['server_key']	. $SG['pp_del'];
		$reply .= "recognition"	. $SG['kv_del']	. "0"					. $SG['pp_del'];
		$reply .= "manid" 		. $SG['kv_del'] . $manid				. $SG['pp_del'];
		$reply .= "modnum" 		. $SG['kv_del'] . $modnum				. $SG['pp_del'];
		$reply .= "sn" 			. $SG['kv_del'] . $sn					. $SG['pp_del'];
		
		print $reply;
	

	// .. else sensor is not recognized
	} else {
	}

	
}


?>
