<?php
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");


/* ************************************************************************** */
function processEyeball ($router_ip, $payload, $db) {
	global $SG;

	// Data from API
	$rdid 		= $payload["rdid"];
	$modnum 	= cleanZeroes(strtolower($payload["modnum"]));
	$manid 		= cleanZeroes(strtolower($payload["manid"]));
	$sn 		= cleanZeroes(strtolower($payload["sn"]));
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

		// Let's get this sensor's id
		$query  = "SELECT id_sensor FROM sensor ";
		$query .= "WHERE modnum='$modnum' AND manid='$manid' ";
		$query .= "AND sn='$sn' AND has_mated='y'";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query eb5.");
		$row = mysqli_fetch_assoc($result);
		$id_sensor = $row['id_sensor'];

		// Let's also set the sensor's status to "offline".  This takes care
		// of the possiblity of a sensor trying to reconnect to the network 
		// while we're under the impression it's still connected. 
		$query  = "UPDATE sensor SET status='offline' ";
		$query .= "WHERE modnum='$modnum' AND manid='$manid' ";
		$query .= "AND sn='$sn' AND has_mated='y'";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query eb6.");
		
		// Build the Peck
		$reply = appendToPayload($reply, "dt", 			"1");
		$reply = appendToPayload($reply, "ip", 			"");
		$reply = appendToPayload($reply, "sid", 	 	$server_id);
		$reply = appendToPayload($reply, "recognition",	"0");
		$reply = appendToPayload($reply, "manid", 		$manid);
		$reply = appendToPayload($reply, "modnum", 		$modnum);
		$reply = appendToPayload($reply, "sn", 			$sn);
		
		// Send Peck
		xmitToRouter($reply, $router_ip);

		// Sleep for 250 ms after Peck before sending Squawk. 
		usleep(250000);		

		// Now we start squawking.  SQUAWK!!!
		generateSquawk ($rdid, $id_sensor, $router_ip, $db);
	

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
			#$msg .= "(Sensor: $id_sensor) ";
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
	$sig_id_a	= cleanZeroes($payload["sida"]);
	$classa		= cleanZeroes($payload["classa"]);
	$dira		= cleanZeroes($payload["dira"]);
	$labela		= $payload["labela"] != "" ?  $payload["labela"] : "-";
	$unitsa		= $payload["unitsa"] != "" ?  $payload["unitsa"] : "&nsbp;";
	$sig_id_b	= cleanZeroes($payload["sidb"]);
	$classb		= cleanZeroes($payload["classb"]);
	$dirb		= cleanZeroes($payload["dirb"]);
	$labelb		= $payload["labelb"] != "" ?  $payload["labelb"] : "-";
	$unitsb		= $payload["unitsb"] != "" ?  $payload["unitsb"] : "&nsbp;";
	$additional	= cleanZeroes($payload["additional"]);

	// First we gotta find out the sensor's information from the pipeline;
	$query = "SELECT * FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query ($db, $query) or die ("Can't execute query pc1.");
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
	$query .= " VALUES ('$id_sensor', '$sig_id_a', '$classa', '$dira', '$labela', '$unitsa')"; 
	mysqli_query ($db, $query) or die ("Can't execute query pc2.");

	$msg  = "[$router_ip] - Peacock: Added '$labela' for sensor $id_sensor. ";
	addToLog($msg);

	// Now, do we have I/O 'B'?
	if ( (hexdec($sig_id_b) !=  253) && ($sig_id_b != "") ) {  // 0xFd = 253

		//print "sig_id_b is $sig_id_b or as a decimal it's " . hexdec($sig_id_b) . " \n\n";

		$query  = "INSERT INTO io (id_sensor,sig_id,class,direction,label,units)";
		$query .= " VALUES ('$id_sensor', '$sig_id_b', '$classb', '$dirb', '$labelb', '$unitsb')"; 
		mysqli_query ($db, $query) or die ("Can't execute query pc3.");

		$msg  = "[$router_ip] - Peacock: Added '$labelb' for sensor $id_sensor. ";
		addToLog($msg);
	}

	// Is this the last Peacock? If so, then it's time to Nest!! Woo!!!
	if ($additional	== 0) {
		generateNest($router_ip, $rdid, $db);
	// If we have more I/O coming, let's log that we're waiting
	} else {
		$msg  = "[$router_ip] - Additional Peacocks expected for sensor $id_sensor. ";
		addToLog($msg);
	}

} // End processPeacock


// ****************************************

function generateSquawk ($rdid, $id_sensor, $router_ip, $db) {
	global $SG;

	// Beginning the squawk payload
	// all replies should include the originating rdid
	$reply = appendtopayload($SG['ff_del'], "rdid", $rdid);

	// Do we have a server key?
	$query = "SELECT server_key FROM server";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query sq1.");
	$row = mysqli_fetch_assoc($result);
	$server_key = $row['server_key'];


	// If we don't have a server key...
	if ($server_key == "" ) {
		
		$dt =  "12";

		// No challenge
		$challenge = "";

	// ...else we do have a server key;
	} else {

		$dt = "11";

		$challenge = generaterandomhash($SG['skl']);
	}

	$reply = appendtopayload($reply, "dt", 		$dt);
	$reply = appendtopayload($reply, "data", 	"$challenge");


	// log it
	$msg  = "[$router_ip] - Eyeball: Recognized sensor. ";
	$msg .= "Squawking to commence ";
	$msg .= "(Sensor: $id_sensor). ";
	if ($challenge == "") {
		$msg .= "No challenge. ";
	} else {
		$msg .= "Challenge = $challenge.";
	}
	addtolog($msg);


	xmittorouter($reply, $router_ip);

	// update pipeline
	updatepipeline ($rdid, $id_sensor, $router_ip, 'Squawk1', $challenge);

	// debugging only
	if ( ($challenge != "") && ($SG['debug'] == true ) ) {
		
		$challenge_response = countones(sgxor($server_key, $challenge));

		$reply = appendtopayload($SG['ff_del'], "rdid", $rdid);
		$reply = appendtopayload($reply, "dt", 		"__debug__");
		$reply = appendtopayload($reply, "data", 	"$challenge_response");

		xmittorouter($reply, $router_ip);
	}

} // End generatesquawk()


// ****************************************


function processSquawkSensorReply($router_ip, $payload, $db) {
	global $SG;

	$rdid 				= $payload['rdid'];
	$dt   				= $payload['dt'];
	$challenge_response = $payload['data'];

	// Get id_sensor
	$query = "SELECT id_sensor FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query sq3.");
	$row = mysqli_fetch_assoc($result);
	$id_sensor = $row['id_sensor'];

	// Do we have a server key?
	$query = "SELECT server_key FROM server";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query sq4.");
	$row = mysqli_fetch_assoc($result);
	$server_key = $row['server_key'];


	// If we don't have a server key then there's no response to check
	if ($server_key == "" ) {

		// If the sensor's not going to send a challenge then we nest
		if ($dt == "15") {
			generateNest($router_ip, $rdid, $db);

		} else {

			$msg  = "[$router_ip] - Squawk: ";
			$msg .= "Awaiting sensor's challenge. ";
			$msg .= "(Sensor: $id_sensor). ";
			addtolog($msg);

			// If the sensor is going to send a challenge then update the pipeline
			// to signify that we're ready for it.
			updatePipeline ($rdid, $id_sensor, $router_ip, 'Squawk2');
		}
		
		

	// ...else, check the challenge response
	} else {

		// Get the challenge we sent the sensor
		$query = "SELECT workspace FROM pipeline WHERE rdid='$rdid'";
		$result = mysqli_query($db, $query) 
			or die ("error: couldn't execute query sq2.");
		$row = mysqli_fetch_assoc($result);
		$challenge = $row['workspace'];

		
		// So the right answer is (DAH DAH DAHHHHH):
		$correct_response = countones(sgxor($server_key, $challenge));
	
		// Was the sensor right?
		if ($correct_response == $challenge_response) {

			// If the sensor's not going to send a challenge then we nest
			if ($dt == "15") {
				// Log it
				$msg  = "[$router_ip] - Squawk: Successful authenticated. ";
				$msg .= "Permission to Nest granted. ";
				$msg .= "(Sensor: $id_sensor). ";
				addtolog($msg);

				generateNest($router_ip, $rdid, $db);



			} else { 
				// Log it
				// Log it
				$msg  = "[$router_ip] - Squawk: Successful authenticated. ";
				$msg .= "Awaiting sensor's challenge. ";
				$msg .= "(Sensor: $id_sensor). ";
				addtolog($msg);

				// If the sensor is going to send a challenge then update the 
				// pipeline to signify that we're ready for it.
				updatePipeline ($rdid, $id_sensor, $router_ip, 'Squawk2');
			}
		 

		// else the sensor was wrong. :(
		} else {

			// Beginning the Squawk payload
			// All replies should include the originating rdid
			$reply = appendtopayload($SG['ff_del'], "rdid", $rdid);

			$reply = appendtopayload($reply, "dt", 		"1b");
			$reply = appendtopayload($reply, "data", 	"");

			// Log it
			$msg  = "[$router_ip] - Squawk: Sensor failed authentication. ";
			$msg .= "Bye bye. ";
			$msg .= "(Sensor: $id_sensor). ";
			addtolog($msg);

			// Send it
			xmittorouter($reply, $router_ip);

			// Bye sensor :(
			deleteFromPipelineByRdid($rdid, $db);
		}

	}

} // End processSquawkSensorReply();


// ****************************************


function processSquawkSensorChallenge($router_ip, $payload, $db) {
	global $SG;

	$rdid 		= $payload['rdid'];
	$dt   		= $payload['dt'];
	$challenge	= $payload['data'];

	// OK, before we get started we need to realize that this payload (dt=0x17)
	// comes immediately after the previous (dt=0x16) without any server 
	// interaction between.  Hence, network issues may cause the server to see
	// 0x17 before 0x16, which is bad.  If that's the case we'll just sit and
	// wait until we see that 0x16 is has set the pipeline status to 'Squawk2'
	// or until the sensor times out of the pipeline.
	$keep_waiting = TRUE;
	while ( $keep_waiting == TRUE ) {
		$query = "SELECT COUNT(*) AS count  FROM pipeline WHERE rdid='$rdid'";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query sq7.");
		$row = mysqli_fetch_assoc($result);


		// If the sensor has timed out of the pipeline then we just quit
		if ($row['count'] < 1 ) {
			die ("Pipeline timeout during Squawk.");
		}

		// Now check for Squawk2
		$query  = "SELECT COUNT(*) AS count  FROM pipeline ";
		$query .= "WHERE rdid='$rdid' AND latest_tx='Squawk2'";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query sq7.");
		$row = mysqli_fetch_assoc($result);


		// If the sensor has timed out of the pipeline then we just quit
		if ($row['count'] > 0 ) {
			$keep_waiting = FALSE;
		}

		// Sleep for 250 ms
		usleep(250000);		

	}


	// Get id_sensor
	$query = "SELECT id_sensor FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query sq5.");
	$row = mysqli_fetch_assoc($result);
	$id_sensor = $row['id_sensor'];

	// Get the sensor's key
	$query = "SELECT sensor_key FROM sensor WHERE id_sensor='$id_sensor'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query sq6.");
	$row = mysqli_fetch_assoc($result);
	$sensor_key = $row['sensor_key'];


	// If we don't have a sensor key, then the sensor shouldn't have sent a
	// challenge.  So we're just to kick it off the network.
	if ($sensor_key == "") {
		// THIS SHOULD A CHIRP OF TYPE 0X25
		// THIS IS NOT DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		print "id_sensor = $id_sensor\n";
		die ("oops...");
	}


	// So the right answer is (DAH DAH DAHHHHH):
	$response = countones(sgxor($sensor_key, $challenge));

	// All replies should include the originating rdid
	$reply = appendtopayload($SG['ff_del'], "rdid", $rdid);

	$reply = appendtopayload($reply, "dt", 		"1c");
	$reply = appendtopayload($reply, "data", 	$response);
	
	// Log it
	$msg  = "[$router_ip] - Squawk: Replied to sensor challenge. ";
	$msg .= "Awaiting sensor's acceptance. ";
	$msg .= "(Sensor: $id_sensor). ";
	addtolog($msg);

	// If the sensor is going to send a challenge then update the 
	// pipeline to signify that we're ready for it.
	updatePipeline ($rdid, $id_sensor, $router_ip, 'Squawk3');
		 

	// Send it
	xmittorouter($reply, $router_ip);

} // End processSquawkSensorChallenge()


// ****************************************


function processSquawkAcceptsChallenge($router_ip, $payload, $db) {
	global $SG;

	$rdid 				= $payload['rdid'];
	$dt   				= $payload['dt'];
	$challenge_response = $payload['data'];


	generateNest($router_ip, $rdid, $db);

} // End processSquawkAcceptsChallenge()


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
			or die ("Error: Couldn't execute query ne7.");
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
	$query  = "UPDATE sensor ";
	$query .= "SET has_mated='y', status='online', rdid='$rdid', ";
	$query .= "router_ip='$router_ip'";
	$query .= "WHERE id_sensor='$id_sensor'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ne1.");

}


/* ************************************************************************** */


function processChirpData($router_ip, $payload, $db) {
	global $SG;

	$rdid 	= $payload['rdid'];
	$sig_id	= cleanZeroes(strtolower($payload['sid']));
	$data 	= strtolower($payload['data']);

	// Log it
	//$msg  = "DEBUG -- Chirp data:  ";
	//addtolog($msg);

	// Do we have an online sensor using this rdid?...
	$query  = "SELECT COUNT(*) as count FROM sensor ";
	$query .= "WHERE rdid='$rdid' AND status='online' OR status='stale'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query pcd1.");
	$row = mysqli_fetch_assoc($result);
	$count = $row['count'];

	// ...If we don't, then we give up.
	if ( $count != 1 ) 
		die ("Error: sensor not online");
	
	// Now let's get the sensor data
	$query = "SELECT * FROM sensor WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) 
		or die ("Couldn't execute query pcd2.");
	$row = mysqli_fetch_assoc($result);
	$id_sensor 	= $row['id_sensor'];

	// Finally we update
	$query  = "UPDATE io SET value='$data' ";
	$query .= "WHERE id_sensor='$id_sensor' AND sig_id='$sig_id'";
	mysqli_query($db, $query) or die ("Couldn't execute query pcd3.");

	// Log it
	$msg  = "Update signal ($sig_id) from ($id_sensor): $data. ";
	addtolog($msg);

	// Last thing to do is send this data to the trigger manager to see if
	// we've set off a trigger. 
	checkTriggers ($id_sensor, $sig_id, $data);


} // End processChirpData()


/* ************************************************************************** */


function dropSensorFromNetwork($router_ip, $payload, $db) {
	global $SG;

	$rdid 	= $payload['rdid'];
	$sig_id	= strtolower($payload['status']);
	$data 	= strtolower($payload['data']);

	// Take sensor offline within database
	takeSensorOffline ($rdid);

	// Send the "disconnect sensor from network" command
	$reply = appendToPayload($SG['ff_del'], "rdid", 	$rdid);
	$reply = appendToPayload($reply, 		"dt", 		"25");
	$reply = appendToPayload($reply, 		"data", 	"");

	xmitToRouter($reply, $router_ip);

} // End dropSensorFromNetwork()


/* ************************************************************************** */


function updateSensorStatus($router_ip, $payload, $db) {
	global $SG;

	$rdid 				= $payload['rdid'];
	$new_status 		= strtolower($payload['status']);


	// If the update is 'offline' then just do it
	if ( $new_status == 'offline' ) {
		takeSensorOffline ($rdid);
		
	// If the update is 'online' or 'stale' we have to first make sure that the
	// sensor is also either 'online ' or 'stale'.  In other words, a sensor
	// can't go from 'offline' to either of other two conditions.  It would need
	// to re-Squawk to get back on the network. 
	} else {

		// OK, so what's the current status?
		$query = "SELECT id_sensor, status FROM sensor WHERE rdid='$rdid'";
		$result = mysqli_query($db, $query) 
			or die ("Couldn't execute query uss1.");
		$row = mysqli_fetch_assoc($result);

		$id_sensor = $row['id_sensor'];
		$status = $row['status'];

		// If sensor isn't offline, then go ahead and update
		if ( ($status == "online") OR ($status == 'stale') ) {
			$query  = "UPDATE sensor SET status='$new_status' ";
			$query .= "WHERE rdid='$rdid'";
			$result = mysqli_query($db, $query) 
				or die ("Couldn't execute query uss2.");

			// Log it
			$msg  = "Sensor ($id_sensor) now $new_status. ";
			addtolog($msg);

		}

	}

	// Now we tell the router that we've updated the status successfully
	$reply = appendToPayload($SG['ff_del'], "rdid", 	$rdid);
	$reply = appendToPayload($reply, 		"dt", 		"fd");
	$reply = appendToPayload($reply, 		"status", 	"ack");

	xmitToRouter($reply, $router_ip);


} // End updateSensorStatus()


/* ************************************************************************** */

?>
