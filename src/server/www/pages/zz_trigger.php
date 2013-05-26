<?
// Checks to see if the $data received from the $id_src_sensor's $id_src_signal
// trips a trigger.  If it does, the response is initiated.
function checkTriggers ($id_src_sensor, $sig_id, $data) {

	$db = returnDatabaseConnection();

	// First we have to get the $id_signal for the this $id_src_sensor based
	// on that signal's sig_id (as determined by the sensor). Confusing, I know.
	$query  = "SELECT id_io FROM io WHERE ";
	$query .= "id_sensor='$id_src_sensor' AND sig_id='$sig_id'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ct1.");
	$row = mysqli_fetch_assoc($result);
	$id_src_signal = $row['id_io'] or die ("Strange error in triggers...\n");
	
	// Next, do we have any triggers with this sensor/signal as the source?
	$query  = "SELECT COUNT(*) AS count FROM triggers WHERE ";
	$query .= "id_src_sensor='$id_src_sensor' AND id_src_signal='$id_src_signal'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ct2.");
	$row = mysqli_fetch_assoc($result);

	// If no triggers found, then we're done here.
	if ($row['count'] < 1) 
		return;

	// Now, get the triggers for this sensor/signal.
	$query  = "SELECT * FROM triggers WHERE ";
	$query .= "id_src_sensor='$id_src_sensor' AND id_src_signal='$id_src_signal'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query ct3.");
	
	while ($row = mysqli_fetch_assoc($result) ) {
		$id_dest_sensor = $row['id_dest_sensor'];
		$id_dest_signal = $row['id_dest_signal'];
		$trigger_type = $row['trigger_type'];
		$trigger_value = $row['trigger_value'];
		$dest_type = $row['dest_type'];
		$dest_value = $row['dest_value'];

		// If we have a trigger condition, then we look at the $dest_type and
		// send the appropriate Chirp
		if ( testForTrigger($trigger_type, $trigger_value, $data) ) { 

			// Now what is the destination data (what we're sending out)
			// supposed to be?
			switch ($dest_type) {
				case "digital_0":
					sendTriggerResponse ($id_dest_sensor, 
										 $id_dest_signal, 
										 0);
					break;

				case "digital_1":
					sendTriggerResponse ($id_dest_sensor, 
										 $id_dest_signal, 
										 1);
					break;

				case "trigger_value":
					sendTriggerResponse ($id_dest_sensor, 
										 $id_dest_signal, 
										 $data);
					break;

				case "user_value":
					sendTriggerResponse ($id_dest_sensor, 
										 $id_dest_signal, 
										 $dest_value);
					break;
			}
		}

	}
} // End checkTriggers()
	

/* ************************************************************************** */


// Checks to ensure that the destination sensor is online, gets the sig_id for
// this particular $id_dest_signal, and then hands everything off to the
// sendChirpData() function (in zz_sensor.php). 
function sendTriggerResponse ($id_dest_sensor, $id_dest_signal, $dest_value) {
	
	$db = returnDatabaseConnection();

	// Get data for the destination sensor
	$query = "SELECT * FROM sensor WHERE id_sensor='$id_dest_sensor'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query str1.");
	$row = mysqli_fetch_assoc($result);

	$status = $row['status'];
	$rdid = $row['rdid'];
	$router_ip = $row['router_ip'];

	// If the destination sensor is  offline (or for whatever reason don't have 
	// an IP address or rdid) then we're done.
	if ( ($status == 'offline') or ($rdid == '') or ($router_ip == '') )
		return;
	
	// Now we just need to get the sig_id
	$query  = "SELECT sig_id FROM io WHERE ";
	$query .= "id_sensor='$id_dest_sensor' AND id_io='$id_dest_signal'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query str2.");
	$row = mysqli_fetch_assoc($result);

	$sig_id = $row['sig_id'];

	sendChirpData ($rdid, $sig_id, $dest_value);
}


/* ************************************************************************** */

// Going through all the options for a valid trigger generated some ugly code. 
// To keep things clean, I've moved it into a separate function.  This function
// looks at the $trigger_type and compares it with the $data.  If a trigger
// condition is met it returns true, otherwise it returns false.
function testForTrigger ($trigger_type, $trigger_value, $data) {

	if (($trigger_type == "digital_0") and ($data == 0)) 
		return true;

	if (($trigger_type == "digital_1") and ($data == 1)) 
		return true;

	if ($trigger_type == "digital_change")
		return true;

	if (($trigger_type == "equal") and ($data == $trigger_value))
		return true;

	if (($trigger_type == "greater_than") and ($data > $trigger_value))
		return true;

	if (($trigger_type == "less_than") and ($data < $trigger_value))
		return true;

	return false;
} // End function testForTrigger ()

?>
