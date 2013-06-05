<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Last_tx code to english descriptions
$tx_definitions['Sing'] = "Sing has been sent (with or without key), awaiting Mock.";
$tx_definitions['Peck'] = "Awaiting permission to mate with sensor.";
$tx_definitions['Mock'] = "Waiting for sensor to Peacock.";
$tx_definitions['Squawk1'] = "Initial Squawk, no server challenge.";
$tx_definitions['Squawk2'] = "Server awaiting sensor's challenge.";
$tx_definitions['Squawk3'] = "Server responded to sensor's challenge.";

$db = returnDatabaseConnection();


// Now, let's see what's left. 
$query = "SELECT COUNT(*) AS count FROM pipeline";
$result = mysqli_query($db, $query) or die ("Error: Can't query log data.");
$row = mysqli_fetch_assoc($result);


# If there's nothing in the 'pipeline' table...
if ( $row['count'] == 0 ) {
	$msg = "<i>There is currently nothing in the pipeline.</i>";


# ...else, there is data in the 'log' table...
} else  {

	# Table header
	$msg  = "<table class=\"sansgrid\">\n";
	$msg .= "<tr>\n";
	$msg .= "<th width=\"100px\">Router IP</th>\n";
	$msg .= "<th width=\"100px\">rdid</th>\n";
	$msg .= "<th width=\"100px\">id_sensor</th>\n";
	$msg .= "<th>Last TX</th>\n";
	$msg .= "<th width=\"200px\">Time</th>\n";
	$msg .= "</tr>\n";

	# Let's get the data...
	$query = "SELECT * FROM pipeline ORDER BY last_update DESC";
	$result = mysqli_query($db, $query) or die ("Error: Can't get pipeline data.");

	# ...and populate our table
	while ( $row = mysqli_fetch_assoc($result) ) {

		// This is just to get the textual description for each 'latest_tx' entry
		$idx = $row['latest_tx'];
		$tx_def = $tx_definitions["$idx"];

		$msg .= "<tr>\n";
		$msg .= "<td class=\"alignc\">" . $row['router_ip'] . "</td>\n";
		$msg .= "<td class=\"alignc\">" . $row['rdid'] . "</td>\n";
		$msg .= "<td class=\"alignc\">" . $row['id_sensor'] . "</td>\n";
		$msg .= "<td class=\"alignc\" title=\"$tx_def\"> " . $row['latest_tx'] . "</td>\n";
		$msg .= "<td class=\"alignc\">" . $row['last_update'] . "</td>\n";
		$msg .= "</tr>\n";
	}

	$msg .= "</table>\n";
}


?>
<html>
<head>
<title>Pipeline - <? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
<? print returnRefresh(); ?>
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Pipeline</h2>

<?  print $msg; ?>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
