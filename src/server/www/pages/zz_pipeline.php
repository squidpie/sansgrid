<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Updates the pipeline...
// We should get either an rdid (during mating) or an id_sensor (during squawking). 
function updatePipeline ($rdid, $id_sensor, $router_ip, $tx, $workspace="") {
	$db = returnDatabaseConnection();


	// Mating
	if ($rdid != "") {

		// Delete the current entry before adding a new one for this rdid.
		deleteFromPipelineByRdid ($rdid, $db);

		$query  = "INSERT INTO pipeline (rdid, id_sensor, router_ip, latest_tx, workspace) ";
		$query .= "VALUES ('$rdid', '$id_sensor', '$router_ip', '$tx', '$workspace');";
		$result = mysqli_query($db, $query) 
			or die ("Error: Couldn't execute query pl1.");

	// Squawking
	} else {
		print "\nPIPELINE-SQUAWKING STUFF STILL ISN'T WRITTEN!!!!!!!\n";
	}
	
}

// ****************************************

function deleteFromPipelineByRdid ($rdid, $db) {

	// Notice that we're not checking if $rdid exists.  MySQL shouldn't generate an error
	// if it doesn't.  There's just nothing to delete. 
	$query = "DELETE FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query pl2.");
}

// ****************************************

// This function will go through every possible 'last_tx' condition and blindly
// delete any processes which are considered expired (times set in config file). 
function cleanPipeline() {
	global $SG;

	$db = returnDatabaseConnection();

	// Find out who's late based on the 'maxtime' settings in config.php
	foreach ($SG['maxtime'] as $latest_tx => $maxtime) {
		$query  = "SELECT * FROM pipeline ";
		$query .= "WHERE last_update < DATE_SUB(NOW(), INTERVAL $maxtime second) ";
		$query .= "AND latest_tx='$latest_tx'";
		$result = mysqli_query($db, $query) 
			or die ("Error: Can't execute query pl3 ($latest_tx).");

		while ($row = mysqli_fetch_assoc($result)) {
			$id_sensor 	= $row['id_sensor'];
			$rdid 		= $row['rdid'];
			$router_ip 	= $row['router_ip'];

			// Remove sensor I/O from 'io' table
			// !!!!!!!!!!!!!! THIS ISN'T DONE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!111

			// Remove sensor from 'sensor' table IF we've never mated before
			$query  = "DELETE FROM sensor  ";
			$query .= "WHERE id_sensor='$id_sensor' AND has_mated='n'";
			mysqli_query($db, $query) 
				or die ("Error: Can't execute query pl4 ($latest_tx).");

			// Remove sensor from 'pipeline' table
			$query  = "DELETE FROM pipeline WHERE id_sensor='$id_sensor' ";
			mysqli_query($db, $query) 
				or die ("Error: Can't execute query pl5 ($latest_tx).");

			// Send the "disconnect sensor from network" command
			$reply = appendToPayload($SG['ff_del'], "rdid", 	$rdid);
			$reply = appendToPayload($reply, 		"dt", 		"25");
			$reply = appendToPayload($reply, 		"data", 	"");

			xmitToRouter($reply, $router_ip);

			$msg = "Sensor $id_sensor has expired from pipeline.";
			addToLog($msg);

		}
	}

}

?>


