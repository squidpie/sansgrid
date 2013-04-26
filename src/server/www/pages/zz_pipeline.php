<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Updates the pipeline...
// We should get either an rdid (during mating) or an id_sensor (during squawking). 
function updatePipeline ($rdid, $id_sensor, $router_ip, $tx) {
	$db = returnDatabaseConnection();


	// Mating
	if ($rdid != "") {

		// Delete the current entry before adding a new one for this rdid.
		deleteFromPipelineByRdid ($rdid, $db);

		$query  = "INSERT INTO pipeline (rdid, id_sensor, router_ip, latest_tx) ";
		$query .= "VALUES ('$rdid', '$id_sensor', '$router_ip', '$tx');";
		$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query pl1.");

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
		$query  = "SELECT id_sensor FROM pipeline ";
		$query .= "WHERE last_update < DATE_SUB(NOW(), INTERVAL $maxtime second) ";
		$query .= "AND latest_tx='$latest_tx'";
		$result = mysqli_query($db, $query) 
			or die ("Error: Can't execute query pl3 ($latest_tx).");

		while ($row = mysqli_fetch_assoc($result)) {
			$id_sensor = $row['id_sensor'];

			// Remove sensor I/O from 'io' table
			// !!!!!!!!!!!!!! THIS ISN'T DONE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!111

			// Remove sensor from 'sensor' table
			$query  = "DELETE FROM sensor WHERE id_sensor='$id_sensor' ";
			mysqli_query($db, $query) or die ("Error: Can't execute query pl4 ($latest_tx).");

			// Remove sensor from 'pipeline' table
			$query  = "DELETE FROM pipeline WHERE id_sensor='$id_sensor' ";
			mysqli_query($db, $query) or die ("Error: Can't execute query pl5 ($latest_tx).");

		}
	}

}

?>


