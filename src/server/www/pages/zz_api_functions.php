<?php
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");


/* ************************************************************************** */
function processEyeball ($router_ip, $payload, $db) {
	global $SG;

	// Data from API
	$rdid 		= $payload["rdid"];
	$modnum 	= strtolower($payload["modnum"]);
	$manid 		= strtolower($payload["manid"]);
	$sn 		= strtolower($payload["sn"]);
	$mode 		= $payload["mode"];

	// First thing to do is ensure that this rdid isn't currently in the pipeline
	deleteFromPipelineByRdid ($rdid, $db);

	// All replies should include the originating rdid
	$reply = appendToPayload("", "${SG['ff_del']}rdid", $rdid);
	
	// Do we recognize this sensor?
	$query  = "SELECT COUNT(*) as count FROM sensor ";
	$query .= "WHERE modnum='$modnum' AND manid='$manid' ";
	$query .= "AND sn='$sn' AND has_mated='y'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query eb1.");

	$row = mysqli_fetch_assoc($result);
	$count = $row['count'];

	// Now we generate the appropriate Peck, start by getting the server id.
	$query = "SELECT server_id FROM server";
	$result = mysqli_query ($db, $query) 
		or die ("Error: Couldn't execute query eb2.");
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
		
		xmitToRouter($reply, $router_ip);


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
			$msg .= "serial number&nbsp;=&nbsp;$sn. ";
			$msg .= "(Sensor: $id_sensor) ";
			addToLog($msg);

			xmitToRouter($reply, $router_ip);

		// ...else sensor is ready to mate
		} else {

			// OK, so the sensor is ready to mate. Let's get it added into the
			// database so we can save Eyeball, Mock and Peacock data. 
			$query  = "INSERT INTO sensor (manid, modnum, sn, status) ";
			$query .= "VALUES ('$manid', '$modnum', '$sn', 'offline')";
			$result = mysqli_query($db, $query) 
				or die ("Couldn't execute query eb4.");
			$id_sensor = mysqli_insert_id($db);

			// Is server set to allow automatic mating?
			$query = "SELECT verify_mating FROM server";
			$result = mysqli_query($db, $query) 
				or die ("Couldn't execute query eb3.");
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

				xmitToRouter($reply, $router_ip);

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
				$msg  = "[$router_ip] - Eyeball: New sensor entered network. ";
				$msg .= "Peck complete. Awaiting permission to mate. ";
				$msg .= "Manufacturer's ID&nbsp;=&nbsp;$manid, ";
				$msg .= "model number&nbsp;=&nbsp;$modnum, ";
				$msg .= "serial number&nbsp;=&nbsp;$sn. ";
				$msg .= "(Sensor: $id_sensor) ";
				addToLog($msg);

				xmitToRouter($reply, $router_ip);

				// Update pipeline
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
	$msg  = "[$router_ip] - Eyeball: New sensor entered network. ";
	$msg .= "Peck complete. Sing issued. ";
	$msg .= "Manufacturer's ID&nbsp;=&nbsp;$manid, ";
	$msg .= "model number&nbsp;=&nbsp;$modnum, ";
	$msg .= "serial number&nbsp;=&nbsp;$sn. ";
	$msg .= "(Sensor: $id_sensor) ";
	addToLog($msg);

	xmitToRouter($reply, $router_ip);

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
		$reply = appendToPayload($reply, 		"dt", 	 "NEED_A_DATA_TYPE");

		// You tell em!
		$msg  = "[$router_ip] - Error: Received erroneous Mock. ";
		$msg .= "Sensor kicked off network. ";
		$msg .= "(Sensor: $id_sensor) ";
		addToLog($msg);

		// Umm... now you tell the sensor
		xmitToRouter($reply, $router_ip);

		return;
	}

	// Do we expect a key, and if so did we get one? If yes then let's save it. 
	if ( ($dt == 7) && ($senspubkey != "") ) {
		$query  = "UPDATE sensor SET sensor_key='$senspubkey' ";
		$query .= "WHERE id_sensor='$id_sensor'";
		mysqli_query($db, $query) or die ("Can't execute query mo2.");

		$msg  = "[$router_ip] - Received Mock. Sensor key saved. ";
		$msg .= "(Sensor: $id_sensor) ";
		addToLog($msg);

		updatePipeline ($rdid, $id_sensor, $router_ip, 'Mock');

	// Or was this a Mock with no expected key?
	} else if ($dt == 8) {
		$msg  = "[$router_ip] - Received Mock, no sensor key. ";
		$msg .= "(Sensor: $id_sensor) ";
		addToLog($msg);

		updatePipeline ($rdid, $id_sensor, $router_ip, 'Mock');
	}
	

}


/* ************************************************************************** */
function processPeacock ($router_ip, $payload, $db) {
	global $SG;

	// Data from API
	$rdid 		= $payload["rdid"];
	$dt 		= $payload["dt"];
	$sida		= $payload["sida"];
	$classa		= $payload["classa"];
	$dira		= $payload["dira"];
	$labela		= $payload["labela"];
	$unitsa		= $payload["unitsa"];
	$sidb		= $payload["sidb"];
	$classb		= $payload["classb"];
	$dirb		= $payload["dirb"];
	$labelb		= $payload["labelb"];
	$unitsb		= $payload["unitsb"];
	$additional	= $payload["additional"];

	// First we gotta find out the sensor's information from the pipeline;
	$query = "SELECT * FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query ($db, $query) or die ("Can't execute query pc1.\n$query\n");
	$row = mysqli_fetch_assoc($result);

	// Obviously the router IP in the pipeline should be the same as the one
	// that sent the Mock
	if ($router_ip != $row['router_ip']) 
		die ("Router IP mismatch!");
	
	$latest_tx 	= $row['latest_tx'];
	$id_sensor 	= $row['id_sensor'];


	// If we're receiving a Peacock, then the $last_tx should've been a 'Mock' or
	// a 'Peacock'. If that's not the case then we error out. 
	if ( ($latest_tx != "Mock") && ($latest_tx != "Peacock") ) {

		// !!!!!!!!! THE DATA TYPE OF 'EE' IS NOT VALID!!!!!! NEED AN ERROR DT
		// FIX THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		$reply = appendToPayload($SG['ff_del'], "rdid",	 $rdid);
		$reply = appendToPayload($reply, 		"dt", 	 "NEED_A_DATA_TYPE");

		// You tell em!
		$msg  = "[$router_ip] - Error: Received erroneous Peacock. ";
		$msg .= "Sensor kicked off network. ";
		$msg .= "(Sensor: $id_sensor) ";
		addToLog($msg);

		// Umm... now you tell the sensor
		xmitToRouter($reply, $router_ip);

		return;
	}

	// For sure we have to have I/O 'A', so let's just blindly add that
	$query  = "INSERT INTO io (id_sensor,sig_id, class, direction, label, units)";
	$query .= " VALUES ('$id_sensor', '$sida', '$classa', '$dira', '$labela', '$unitsa')"; 
	mysqli_query ($db, $query) or die ("Can't execute query pc2.");

	$msg  = "[$router_ip] - Peacock: Added '$labela' for sensor $id_sensor. ";
	addToLog($msg);

	// Now, do we have I/O 'B'?
	if ( hexdec($sidb) !=  253) {  // 0xFd = 253

		print "sidb is $sidb or as a decimal it's " . hexdec($sidb) . " \n\n";

		$query  = "INSERT INTO io (id_sensor,sig_id,class,direction,label,units)";
		$query .= " VALUES ('$sidb', '$classb', '$dirb', '$labelb', '$unitsb')"; 
		mysqli_query ($db, $query) or die ("Can't execute query pc3.\n$query\n");

		$msg  = "[$router_ip] - Peacock: Added '$labelb' for sensor $id_sensor. ";
		addToLog($msg);
	}

	// Is this the last Peacock? If so, then it's time to Nest!! Woo!!!
	if ($additional	== 0) {
		generateNest($router_ip, $rdid, $db);
	}

} // End processPeacock

/* ************************************************************************** */

function generateNest ($router_ip, $rdid, $db) {
	global $SG;

	// Before we Nest, we want to see if this manid/modnum combination exists
	// in our compendium.  If it doesn't, we'll go ahead and add it. 
	$query = "SELECT id_sensor FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ne2.");
	$row = mysqli_fetch_assoc($result);
	$id_sensor = $row['id_sensor'];

	$query = "SELECT modnum, manid  FROM sensor WHERE id_sensor='$id_sensor'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ne3.");
	$row = mysqli_fetch_assoc($result);

	$manid 	= $row['manid'];
	$modnum = $row['modnum'];

	// First we check for manid in com (Compendium of Manufacturers)
	$query = "SELECT COUNT(*) AS count FROM com WHERE manid='$manid'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ne4.");
	$row = mysqli_fetch_assoc($result);

	if ( $row['count'] == 0 ) {
		$query = "INSERT INTO com (manid) VALUES ('$manid')";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query ne5.");
	}

	// Next, we check for modnum and manid in cos (Compendium of Sensors)
	$query  = "SELECT COUNT(*) AS count FROM cos "; 
	$query .= "WHERE manid='$manid' AND modnum='$modnum' ";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ne6.");
	$row = mysqli_fetch_assoc($result);

	if ( $row['count'] == 0 ) {
		$query = "INSERT INTO cos (manid, modnum) VALUES ('$manid', '$modnum')";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query ne7.\n\n$query\n\n");
	}


	// Now we can start working on the Nest payload 
	$reply = appendToPayload($SG['ff_del'], "rdid", $rdid);
	$reply = appendToPayload($reply,	 	"dt", 	"10");

	xmitToRouter($reply, $router_ip);

	// Logging
	$msg  = "[$router_ip] - Mating complete!  Permission to Nest granted. ";
	$msg .= "(Sensor: $id_sensor) ";
	addToLog($msg);

	// Delete the sensor from the pipeline
	deleteFromPipelineByRdid ($rdid, $db);

	// Update the sensor to indicate that it has mated and that it's now
	// 'online'
	$query  = "UPDATE sensor SET has_mated='y', status='online' ";
	$query .= "WHERE id_sensor='$id_sensor'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ne1.");

}





?>
