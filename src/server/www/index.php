<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$man_list = "";

// Is this an update?...
if (isset ($_POST["update"])) {

	$id_sensor 	= $_POST["update"];
	$name 		= $_POST["name"][$id_sensor];
	
	$query = "UPDATE sensor SET name='$name' WHERE id_sensor='$id_sensor'";
	mysqli_query($db, $query) or die ("Couldn't execute query 4.");
}


//  Pretty much all of the html for the entire page goes into $page
$page = "<form name=\"sgform\" method=\"post\" action=\"${_SERVER['PHP_SELF']}\" >\n";

// Get list of all sensors we've mated with that are online
$query  = "SELECT * FROM sensor WHERE has_mated='y'";
$query .= " AND status='online' ";
$query .= " ORDER BY manid, modnum";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_sensor 		= $row['id_sensor']; 
	$status 		= $row['status'];
	$manid 			= $row['manid'];		
	$modnum	 		= $row['modnum'];
	$sn 			= $row['sn'];
	$sensor_name	= $row['name'];

	// In case we don't have a name
	$sensor_name = $sensor_name != "" ? $sensor_name : "($id_sensor)";

	// Start fieldset
	$page .= "<fieldset class=\"SGfieldset\">";
	$page .= "<legend>$sensor_name</legend>";
	
	// Do we have any inputs to the sensor?...
	$query  = "SELECT COUNT(*) AS count FROM io ";
	$query .= " WHERE id_sensor='$id_sensor' AND direction='1'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 1.");
	$row2 = mysqli_fetch_assoc($result2);

	//...If so, let's list them. 
	if ($row2['count'] > 0) {
		$page .= "<h4>Sensor Inputs</h4>";

		$page .= "<table width=\"98%\" class=\"sansgrid\">";

		$query  = "SELECT * FROM io ";
		$query .= " WHERE id_sensor='$id_sensor' AND direction='1'";
		$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 1.");
		while ($row2 = mysqli_fetch_assoc($result2)) {

			$label = $row2['label'];
			$units = $row2['units'];

			$page .= "<tr>\n";
			$page .= "\t<th width=\"150px\">$label:</th>\n";
			$page .= "\t<td>\n";
			$page .= "\t\t<input type=\"text\" name=\"value[$id_sensor]\" size=\"15\" class=\"sgtext\">&nbsp;$units";
			$page .= "\t</td>\n";
			$page .= "\t<td width=\"6em\" style=\"text-align:center;\">\n";
			$page .= "\t\t<button type=\"submit\" "; 
			$page .= 	"value=\"$id_sensor\" ";
			$page .= 	"name=\"send\">Send</button>\n";
			$page .= "\t</td>\n";
		}

		$page .= "</table>";
	}

	// Do we have any outputs from the sensor?...
	$query  = "SELECT COUNT(*) AS count FROM io ";
	$query .= " WHERE id_sensor='$id_sensor' AND direction='0'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 1.");
	$row2 = mysqli_fetch_assoc($result2);

	//...If so, let's list them. 
	if ($row2['count'] > 0) {
		$page .= "<h4>Sensor Outputs</h4>";

		$page .= "<table width=\"98%\" class=\"sansgrid\">";

		$query  = "SELECT * FROM io ";
		$query .= " WHERE id_sensor='$id_sensor' AND direction='0'";
		$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 1.");
		while ($row2 = mysqli_fetch_assoc($result2)) {

			$label = $row2['label'];
			$units = $row2['units'];
			$value = $row2['value'] != "" ? $row2['value'] : "-";

			$page .= "<tr>\n";
			$page .= "\t<th width=\"150px\">$label:</th>\n";
			$page .= "\t<td>$value $units</td>\n";
		}

		$page .= "</table>";
	}

	// End fieldset
	$page .= "</fieldset>";
}

$page .= "</form>";


?>
<html>
<head>
<title><? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Online Sensors</h2>
<? print $page; ?>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
