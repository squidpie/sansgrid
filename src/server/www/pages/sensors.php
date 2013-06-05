<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$man_list = "";

// Is the page-load a response to a form entry?
if ( isset ($_POST["come_from"])) {

	// Is this an update?...
	if (isset ($_POST["update"])) {

		$id_sensor = $_POST["update"];
		$name 	= $_POST["name"][$id_sensor];
		
		$query = "UPDATE sensor SET name='$name' WHERE id_sensor='$id_sensor'";
		mysqli_query($db, $query) or die ("Couldn't execute query 4.");
	}

	// ... or is this a delete?  
	if (isset ($_POST["delete"])) {
		$id_sensor = $_POST["delete"];
		deleteSensorByID($id_sensor);
	}
}

//  Pretty much all of the html for the entire page goes into $page
$page = "<form name=\"sgform\" method=\"post\" action=\"${_SERVER['PHP_SELF']}\" >\n";

// Get list of all sensors we've mated with that are online
$query  = "SELECT * FROM sensor WHERE has_mated='y' ORDER BY manid, modnum";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_sensor 		= $row['id_sensor'];
	$status 		= $row['status'];
	$manid 			= $row['manid'];
	$modnum	 		= $row['modnum'];
	$sn 			= $row['sn'];
	$sensor_name	= $row['name'];

	// Do we have a name for this manid?
	$query = "SELECT name FROM com WHERE manid='$manid'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 2.");
	$row2 = mysqli_fetch_assoc($result2);

	$man_name = $row2['name'] != "" ? $row2['name'] : $manid;

	// Do we have a name for this model (manid/modnum)?
	$query = "SELECT name FROM cos WHERE manid='$manid' AND modnum='$modnum'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 3.");
	$row2 = mysqli_fetch_assoc($result2);

	$mod_name = $row2['name'] != "" ? $row2['name'] : $modnum;


	// Table header
	$page .= "<input type=\"hidden\" name=\"come_from\" value=\"hey_there\">\n";
	$page .= "<table width=\"98%\" class=\"sansgrid\">";
	//$page .= "<tr>\n";
	//$page .= "\t<th width=\"20em\">($id_sensor)</th>\n";
	//$page .= "</tr>\n";
	$page .= "<tr>\n";
	$page .= "\t<th width=\"20em\">Manufacturer:</th>\n";
	$page .= "\t<td colspan=\"2\" title=\"manid: $manid\" >$man_name</td>\n";
	$page .= "</tr>\n";
	$page .= "<tr>\n";
	$page .= "\t<th>Model:</th>\n";
	$page .= "\t<td title=\"modnum: $modnum\" colspan=\"2\">$mod_name</td>\n";
	$page .= "</tr>\n";
	$page .= "<tr>\n";
	$page .= "\t<th title=\"ID: $id_sensor\" >Name:</th>\n";
	$page .= "\t<td>\n";
	$page .= "\t\t<input type=\"text\" name=\"name[$id_sensor]\" value=\"$sensor_name\" size=\"30\" class=\"sgtext\">\n";
	$page .= "\t</td>\n";
	$page .= "\t<td width=\"6em\" style=\"text-align:center;\">\n";
	$page .= "\t\t<button type=\"submit\" "; 
	$page .= 	"value=\"$id_sensor\" ";
	$page .= 	"name=\"update\">Update</button>\n";
	$page .= "\t</td>\n";
	$page .= "</tr>\n";
	$page .= "<tr>\n";
	$page .= "\t<th>Serial Number</th>\n";
	$page .= "\t<td  title=\"id_sensor: $id_sensor\" >$sn</td>\n";
	$page .= "\t<td width=\"6em\" style=\"text-align:center;\">\n";
	$page .= "\t\t<button type=\"submit\" "; 
	$page .= 	"value=\"$id_sensor\" ";
	$page .= 	"name=\"delete\">Delete</button>\n";
	$page .= "\t</td>\n";
	$page .= "</tr>\n";
	$page .= "</table>\n";
	$page .= "<br>\n";
	$page .= "<br>\n";

}

$page .= "</form>";


?>
<html>
<head>
<title>Known Sensors - <? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<!--
<fieldset>
<legend>Title</legend>
<input type="radio" name="radio" id="radio"> <label for="radio">Click me</label>
</fieldset>
-->

<h2>Known Sensors</h2>
<? print $page; ?>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
