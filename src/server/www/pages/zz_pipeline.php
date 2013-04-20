<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Updates the pipeline...
// We should get either an rdid (during mating) or an id_sensor (during squawking). 
function updatePipeline ($rdid, $id_sensor, $tx) {
	$db = returnDatabaseConnection();


	// Mating
	if ($rdid != "") {

		// Delete the current entry.
		deleteFromPipelineByRdid ($rdid, $db);

		$query = "INSERT INTO pipeline (rdid, latest_tx) VALUES ('$rdid', '2');";
		$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query pl1.");

	// Squawking
	} else {
		print "\nPIPELINE-SQUAWKING STUFF STILL ISN'T WRITTEN!!!!!!!\n";
	}
	
}

function deleteFromPipelineByRdid ($rdid, $db) {

	// Notice that we're not checking if $rdid exists.  MySQL shouldn't generate an error
	// if it doesn't.  There's just nothing to delete. 
	$query = "DELETE FROM pipeline WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) or die ("Error: Couldn't execute query pl1.");
}


?>


