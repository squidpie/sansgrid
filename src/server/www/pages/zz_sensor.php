<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

/* ************************************************************************** */
//
function deleteSensorByID ($id_sensor) {
	
	$db = returnDatabaseConnection();

	// Delete all I/O associated with sensor
	$query = "DELETE FROM io WHERE id_sensor='$id_sensor'";
	mysqli_query($db, $query) 
		or die ("Can't execute query dsbi1\n");

	// Delete the sensor
	$query = "DELETE FROM sensor WHERE id_sensor='$id_sensor'";
	mysqli_query($db, $query) 
		or die ("Can't execute query dsbi2");
}
/* ************************************************************************** */


/* ************************************************************************** */
// Get's the current refresh rate for auto-refresh'able pages (index, pipeline, /i
// logs).
function takeSensorOffline ($rdid) {

	$db = returnDatabaseConnection();

	// Let's first get the id_sensor for the log
	$query = "SELECT id_sensor FROM sensor WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query tso2.");
	$row = mysqli_fetch_assoc($result);
	$id_sensor = $row['id_sensor'];

	// If we don't see this rdid, then we're done I guess.
	if ( $id_sensor == "")
		return;

	// Log it
	$msg  = "Sensor ($id_sensor) now offline. ";
	addtolog($msg);

	// Now we take the sensor offline and remove the the rdid
	$query  = "UPDATE sensor SET status='offline', rdid='', router_ip='' ";
	$query .= "WHERE id_sensor='$id_sensor'";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query tso3.");


} // End takeSensorOffline()

/* ************************************************************************** */


/* ************************************************************************** */
function sendChirpData ($rdid, $sig_id, $data) {

	global $SG;
	$db = returnDatabaseConnection();

	// Do we have an online sensor with this rdid?
	$query  = "SELECT COUNT(*) AS count FROM sensor ";
	$query .= " WHERE rdid='$rdid' AND (status='online' OR status='stale')";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query scd1.");
	$row = mysqli_fetch_assoc($result);

	if ($row['count'] == 0) 
		return;

	// Get ip address for the sensor
	$query = "SELECT * FROM sensor WHERE rdid='$rdid'";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query scd2.");
	$row = mysqli_fetch_assoc($result);

	$id_sensor = $row['id_sensor'];
	$router_ip = $row['router_ip'];

	// Build payload
	$payload = appendToPayload($SG['ff_del'], 	"rdid",	$rdid);
	$payload = appendToPayload($payload, 		"dt",	"20");
	$payload = appendToPayload($payload, 		"sid", 	$sig_id);
	$payload = appendToPayload($payload, 		"data",	$data);

	// Log it
	$msg  = "Chirp: Sent '$data' to (Sensor: ${id_sensor}[$sig_id])";
	addtolog($msg);

	xmitToRouter ($payload, $router_ip);

}
/* ************************************************************************** */


/* ************************************************************************** */
//
function xmitToRouter ($outbound_payload, $url) {

	//print "This doesn't belong here (zz_tools.php) but: $outbound_payload<br>\n";
	#return;

	# DELETE THIS! THIS IS JUST FOR DEBUGGING!!!!
	if ($url == "10.42.0.1")
		$url = "10.42.0.23";

	#$url="10.42.0.40/API-router.php";
	$url="$url/API-router.php";

	// Make it safe to transmit via http

	// Package it as HTTP data
	$data = "payload=$outbound_payload";

	//Instatiate a curl handle
	$ch = curl_init();

	//Set the router url, number of POST vars, and finally the POST data
	curl_setopt($ch	,CURLOPT_URL, 			$url);
	curl_setopt($ch	,CURLOPT_POST, 			1);		// Passing 1 variable
	curl_setopt($ch	,CURLOPT_POSTFIELDS, 	$data);

	//Do it!
	$junk = curl_exec($ch);		// Ignoring anything returned

	//Did it...
	curl_close($ch);

}
/* ************************************************************************** */


