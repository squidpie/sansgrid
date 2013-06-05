<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$sensor_list = "";

// Is the page-load a response to a form submission?
if ( isset ($_POST['id_sensor']) ) {
	
	$id_sensor = $_POST['id_sensor'];

	// It's possible that sensor $id_sensor was in the pipeline when the user
	// loaded the mate.php page, but has since expired.  So let's double check.
	$query  = "SELECT COUNT(*) AS count FROM pipeline ";
	$query .= "WHERE id_sensor = '$id_sensor' AND latest_tx = 'Peck'";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query 3.");
	$row = mysqli_fetch_assoc($result);

	// Is the mate request still valid?
	if ( $row['count'] == 1 ) {

		// Get sensor data (part 1)
		$query  = "SELECT * FROM pipeline ";
		$query .= "WHERE id_sensor = '$id_sensor' AND latest_tx = 'Peck'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");
		$row = mysqli_fetch_assoc($result);

		$rdid 		= $row['rdid'];
		$router_ip 	= $row['router_ip'];

		// Get sensor data (part 2)
		$query  = "SELECT * FROM sensor WHERE id_sensor = '$id_sensor'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");
		$row = mysqli_fetch_assoc($result);

		$manid 		= $row['manid'];
		$modnum 	= $row['modnum'];
		$sn		 	= $row['sn'];

		generateSing ($rdid, $id_sensor, $manid, $modnum, $sn, $router_ip, $db);
	}


}

// Get list of sensors in pipeline waiting to mate (last tx was a Peck)
$query = "SELECT id_sensor FROM pipeline WHERE latest_tx = 'Peck'";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {
	
	$id_sensor = $row['id_sensor'];

	$query2 = "SELECT * FROM sensor WHERE id_sensor = '$id_sensor'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 2.");
	$row2 = mysqli_fetch_assoc($result2);

	$sensor_list .= "<tr>\n\t<td>";
	$sensor_list .= $row2["manid"];
	$sensor_list .= "</td>\n\t<td>";
	$sensor_list .= $row2["modnum"];
	$sensor_list .= "</td>\n\t<td>";
	$sensor_list .= $row2["sn"];
	$sensor_list .= "</td>\n\t<td>";
	$sensor_list .= "<button type=\"submit\" "; 
	$sensor_list .= 	 "value=\"$id_sensor\" ";
	$sensor_list .= 	"name=\"id_sensor\">Mate!</button>";
	$sensor_list .= "</td>\n</tr>\n";
}


?>
<html>
<head>
<title>Mate - <? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Mating Requests</h2>
<form name="confirm_mate" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<input type="hidden" name="come_from" value="hi_mom">
<table width="98%" class="sansgrid">
<tr>
	<th>Manufacturer</th>
	<th>Model Number</th>
	<th>Serial Number</th>
	<th>Mate?</th>
</tr>
<? print $sensor_list; ?>
</table>
</form>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
