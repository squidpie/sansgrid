<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

function addToLog ($logdata) {
	$db = returnDatabaseConnection();

	if ($logdata != "") {
		$query = "INSERT INTO log (log) VALUES ('$logdata')";
		$result = mysqli_query($db, $query) or die ("Error: Failed writing to log.");
		return TRUE;
	}

	# If we're here that means $logdata was empty.  :(
	return FALSE;
}

function returnLogTable () {
	$db = returnDatabaseConnection();

	$query = "SELECT COUNT(*) AS count FROM log";
	$result = mysqli_query($db, $query) or die ("Error: Can't query log data.");
	$row = mysqli_fetch_assoc($result);


	# If there's nothing in the 'log' table...
	if ( $row['count'] == 0 ) {
		$msg = "<i>No logs found.</i>";


	# ...else, there is data in the 'log' table...
	} else  {

		# Table header
		$msg  = "<table class=\"sansgrid\">\n";
		$msg .= "<tr>\n";
		$msg .= "<th width=\"100px\">ID</th>\n";
		$msg .= "<th>Data</th>\n";
		$msg .= "<th width=\"200px\">Time</th>\n";
		$msg .= "</tr>\n";

		# Let's get the data...
		$query = "SELECT * FROM log ORDER BY time DESC";
		$result = mysqli_query($db, $query) or die ("Error: Can't get log data.");

		# ...and populate our table
		while ( $row = mysqli_fetch_assoc($result) ) {
			$msg .= "<tr>\n";
			$msg .= "<td class=\"alignc\">" . $row['id_log'] . "</td>\n";
			$msg .= "<td>" . $row['log'] . "</td>\n";
			$msg .= "<td class=\"alignc\">" . $row['time'] . "</td>\n";
			$msg .= "</tr>\n";
		}

		$msg .= "</table>\n";
	}

	return $msg;
	
}

?>


