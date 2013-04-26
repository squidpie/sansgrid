<?php
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");


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
	$query .= "AND sn='$sn' AND has_mated='y'";
	$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query EB1.");

	$row = mysqli_fetch_assoc($result);
	$count = $row['count'];

	// Now we generate the appropriate Peck, start by getting the server id.
	$query = "SELECT server_id FROM server";
	$result = mysqli_query ($db, $query) or die ("Error: Couldn't execute query EB2.");
	$row = mysqli_fetch_assoc($result);
	$server_id = $row['server_id'];

	// If sensor is recognized
	if ($count == 1)  {
		
		$reply = appendToPayload($reply, "dt", 			"1");
		$reply = appendToPayload($reply, "ip", 			"");
		$reply = appendToPayload($reply, "sid", 	 	$row['server_id']);
		$reply = appendToPayload($reply, "recognition",	"0");
		$reply = appendToPayload($reply, "manid", 		$manid);
		$reply = appendToPayload($reply, "modnum", 		$modnum);
		$reply = appendToPayload($reply, "sn", 			$sn);
		
		xmitToRouter ($reply);


		// SQUAWKING BELONGS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	

	// .. else sensor is not recognized
	} else {
		// If it's a new sensor, we must mate.  Is the sensor ready to mate?

		//  if sensor not ready to mate...
		if ($mode == 0) {

			$reply = appendToPayload($reply, "dt", 				"1");
			$reply = appendToPayload($reply, "ip", 				"");
			$reply = appendToPayload($reply, "sid", 			$server_id);
			$reply = appendToPayload($reply, "recognition", 	"3");
			$reply = appendToPayload($reply, "manid", 			$manid);
			$reply = appendToPayload($reply, "modnum", 			$modnum);
			$reply = appendToPayload($reply, "sn", 				$sn);

			// Log it
			$msg  = "[$router_ip] - Eyeball: New sensor entered network. ";
			$msg .= "Sensor not ready to mate. ";
			$msg .= "Manufacturer's ID&nbsp;=&nbsp;$manid, ";
			$msg .= "model number&nbsp;=&nbsp;$modnum, ";
			$msg .= "serial number&nbsp;=&nbsp;$sn";
			addToLog($msg);

			xmitToRouter($reply);

		// ...else sensor is ready to mate
		} else {

			// OK, so the sensor is ready to mate. Let's get it added into the
			// database so we can save Eyeball, Mock and Peacock data. 
			$query  = "INSERT INTO sensor (manid, modnum, sn, status) ";
			$query .= "VALUES ('$manid', '$modnum', '$sn', 'offline')";
			$result = mysqli_query($db, $query) or die ("Couldn't execute query EB4.");
			$id_sensor = mysqli_insert_id($db);

			// Is server set to allow automatic mating?
			$query = "SELECT verify_mating FROM server";
			$result = mysqli_query($db, $query) or die ("Couldn't execute query EB3.");
			$row = mysqli_fetch_assoc($result);
			$verify_mating = $row['verify_mating'];

			// If automatic mating is allowed...
			if ($verify_mating == 0) {

				$reply = appendToPayload($reply, "dt", 				"1");
				$reply = appendToPayload($reply, "ip", 				"");
				$reply = appendToPayload($reply, "sid", 			$server_id);
				$reply = appendToPayload($reply, "recognition", 	"1");
				$reply = appendToPayload($reply, "manid", 			$manid);
				$reply = appendToPayload($reply, "modnum", 			$modnum);
				$reply = appendToPayload($reply, "sn", 				$sn);

				xmitToRouter($reply);

				// Sleep for 250 ms after Peck before sending Sing. 
				usleep(250000);		

				// Now we Sing
				generateSing ($rdid, $id_sensor, $manid, $modnum, $sn, $router_ip, $db);


			// ...else automatic mating is not allowed
			} else {
				$reply = appendToPayload($reply, "dt", 			"1");
				$reply = appendToPayload($reply, "ip", 			"");
				$reply = appendToPayload($reply, "sid", 		$server_id);
				$reply = appendToPayload($reply, "recognition",	"4");
				$reply = appendToPayload($reply, "manid", 		$manid);
				$reply = appendToPayload($reply, "modnum", 		$modnum);
				$reply = appendToPayload($reply, "sn", 			$sn);

				// Log it
				$msg  = "[$router_ip] - Eyeball&rarr;Sing: New sensor entered network. ";
				$msg .= "Peck complete. Awaiting permission to mate. ";
				$msg .= "Manufacturer's ID&nbsp;=&nbsp;$manid, ";
				$msg .= "model number&nbsp;=&nbsp;$modnum, ";
				$msg .= "serial number&nbsp;=&nbsp;$sn";
				addToLog($msg);

				xmitToRouter($reply);

				// Update pipeline
				// NEED TO ADD THIS TO PIPELINE AS A SENSOR WAITING TO MATE
				updatePipeline ($rdid, $id_sensor, $router_ip, 'Peck');
			}
			
		}
	}
}

// ****************************************

function generateSing ($rdid, $id_sensor, $manid, $modnum, $sn, $router_ip, $db) {
	global $SG;

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
	$reply = appendToPayload($reply, "servpubkey", 	$server_key);


	// Log it
	$msg  = "[$router_ip] - Eyeball&rarr;Sing: New sensor entered network. ";
	$msg .= "Peck complete. Sing issued. ";
	$msg .= "Manufacturer's ID&nbsp;=&nbsp;$manid, ";
	$msg .= "model number&nbsp;=&nbsp;$modnum, ";
	$msg .= "serial number&nbsp;=&nbsp;$sn";
	addToLog($msg);

	xmitToRouter($reply);

	// Update pipeline
	updatePipeline ($rdid, $id_sensor, $router_ip, 'Sing');
}


/* ************************************************************************** */
function processMock ($router_ip, $payload, $db) {
	global $SG;

	// Data from API
	$rdid 		= $payload["rdid"];
	$dt 		= $payload["dt"];
	$senspubkey	= $payload["senspubkey"];

	// First we gotta find out the sensor's information from the pipeline;
	$query = "SELECT * FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query ($db, $query) or die ("Can't execute query mo1.");
	$row = mysqli_fetch_assoc($result);

	// Obviously the router IP in the pipeline should be the same as the one
	// that sent the Mock
	if ($router_ip != $row['router_ip']) 
		die ("Router IP mismatch!");
	
	$latest_tx 	= $row['latest_tx'];
	$id_sensor 	= $row['id_sensor'];


	// If we're receiving a Mock, then the $last_tx should've been a 'Sing'. If
	// that's not the case then we error out. 
	if ($latest_tx != "Sing") {

		// !!!!!!!!! THE DATA TYPE OF 'EE' IS NOT VALID!!!!!! NEED AN ERROR DT
		// FIX THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		$reply = appendToPayload($SG['ff_del'], "rdid",	 $rdid);
		$reply = appendToPayload($reply, 		"dt", 	 "EE");

		// You tell em!
		$msg  = "[$router_ip] - Error: Received erroneous Mock. ";
		$msg .= "Sensor kicked off network";
		addToLog($msg);

		// Umm... now you tell the sensor
		xmitToRouter($reply);
	}

	// Do we expect a key, and if so did we get one? If yes then let's save it. 
	if ( ($dt == 7) && ($senspubkey != "") ) {
		$query  = "UPDATE sensor SET sensor_key='$senspubkey' ";
		$query .= "WHERE id_sensor='$id_sensor'";
		mysqli_query($db, $query) or die ("Can't execute query mo2.");
	}

}



?>
