<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$man_list = "";

// Is the page-load a response to a form entry?
if ( isset ($_POST["come_from"])) {

	// Is this an update?...
	if (isset ($_POST["update"])) {

		$id_cos = $_POST["update"];
		$name 	= $_POST["name"][$id_cos];
		
		$query = "UPDATE cos SET name='$name' WHERE id_cos='$id_cos'";
		mysqli_query($db, $query) or die ("Couldn't execute query 2.");
	}

	// ... or is this a delete?  
	if (isset ($_POST["delete"])) {

		$id_cos = $_POST["delete"];
		$manid  = $_POST["manid"][$id_cos];
		
		// What's the modnum?
		$query = "SELECT modnum FROM cos WHERE id_cos='$id_cos'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");
		$row = mysqli_fetch_assoc($result);

		$modnum = $row['modnum'];

		// First delete any sensors associated with this mandid & modnum
		$query = "SELECT id_sensor FROM sensor WHERE manid='$manid' AND modnum='$modnum'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query 5.");

		while ($row = mysqli_fetch_assoc($result) ) {
			$id_sensor = $row['id_sensor'];
			deleteSensorByID($id_sensor);
		}

		// Then delete the sensor out of cos
		$query = "DELETE FROM cos WHERE id_cos='$id_cos'";
		mysqli_query($db, $query) or die ("Couldn't execute query 6.");

	}

}


//  Pretty much all of the html for the entire page goes into $page
$page = "<form name=\"sgform\" method=\"post\" action=\"${_SERVER['PHP_SELF']}\" >\n";

// Get list of known manufacturers
$query = "SELECT * FROM com ORDER BY manid";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_com	= $row['id_com'];
	$manid 	= $row['manid'];
	$name  	= $row['name'];

	// Before we go any farther, does this manufacturer have any sensors in
	// our compendium?
	$query = "SELECT COUNT(*) AS count FROM cos WHERE manid='$manid'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 2.");
	$row = mysqli_fetch_assoc($result2);
	if ( $row['count'] == 0 )
		continue;

	// If we don't have a name, then we just use manid
	$name = ( $name == "") ? "Man ID: $manid" : $name;
	$page .= "<h3 class=\"cos\">$name</h3>\n\n";

	// Table header
	$page .= "<input type=\"hidden\" name=\"come_from\" value=\"hey_there\">\n";
	$page .= "<table width=\"98%\" class=\"sansgrid\">";
	$page .= "<tr>\n";
	$page .= "\t<th>Model #</th>\n";
	$page .= "\t<th>Model Name</th>\n";
	$page .= "\t<th>Delete?</th>\n";
	$page .= "</tr>\n";

	// Get list of sensors by this manufacturer
	$query = "SELECT * FROM cos WHERE manid='$manid' ORDER BY modnum";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 3.");

	while ($row2 = mysqli_fetch_assoc($result2)) {

		$id_cos = $row2['id_cos'];
		$modnum = $row2['modnum'];
		$name 	= $row2['name'];

		$page .= "<tr>\n\t<td style=\"text-align:center;\">";
		$page .= $modnum;
		$page .= "</td>\n\t<td>\n";
		$page .= "\t\t<input type=\"hidden\" name=\"manid[$id_cos]\" value=\"$manid\" size=\"30\" class=\"sgtext\">\n";
		$page .= "\t\t<input type=\"text\" name=\"name[$id_cos]\" value=\"$name\" size=\"30\" class=\"sgtext\">\n";
		$page .= "\t\t<button type=\"submit\" "; 
		$page .= 	"value=\"$id_cos\" ";
		$page .= 	"name=\"update\">Update</button>\n";
		$page .= "\t</td>\n\t<td style=\"text-align:center;\">\n";
		$page .= "\t\t<button type=\"submit\" "; 
		$page .= 	"value=\"$id_cos\" ";
		$page .= 	"name=\"delete\">Delete</button>\n";
		$page .= "\t</td>\n</tr>\n";

	}
	$page .= "</table>";

}

$page .= "</form>";


?>
<html>
<head>
<title>Sensor Compendium<? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Compendium of Sensor Types</h2>
<? print $page; ?>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
