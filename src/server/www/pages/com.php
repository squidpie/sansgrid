<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$man_list = "";

// Is the page-load a response to a form entry?
if ( isset ($_POST["come_from"])) {

	// Is this an update?...
	if (isset ($_POST["update"])) {

		$id_com = $_POST["update"];
		$name 	= $_POST["name"][$id_com];
		
		$query = "UPDATE com SET name='$name' WHERE id_com='$id_com'";
		mysqli_query($db, $query) or die ("Couldn't execute query 2.");
	}

	// ... or is this a delete?  
	if (isset ($_POST["delete"])) {

		$id_com = $_POST["delete"];

		// What's the manid?
		$query = "SELECT manid FROM com WHERE id_com='$id_com'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");
		$row = mysqli_fetch_assoc($result);

		$manid = $row['manid'];

		// First delete any sensors associated with this manufacturer
		$query = "SELECT id_sensor FROM sensor WHERE manid='$manid'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query 5.");

		while ($row = mysqli_fetch_assoc($result) ) {
			$id_sensor = $row['id_sensor'];
			deleteSensorByID($id_sensor);
		}

		// Then delete all sensors from the compendium
		$query = "DELETE FROM cos WHERE manid='$manid'";
		mysqli_query($db, $query) or die ("Couldn't execute query 6.");

		// Finally delete the manufacturer
		$query = "DELETE FROM com WHERE id_com='$id_com'";
		mysqli_query($db, $query) or die ("Couldn't execute query 7.");
	}

}

// Get list of known manufacturers
$query = "SELECT * FROM com ORDER BY manid";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_com	= $row['id_com'];
	$manid 	= $row['manid'];
	$name  	= $row['name'];

	
	$man_list .= "<tr>\n\t<td style=\"text-align:center;\">";
	$man_list .= cleanZeroes($manid);
	$man_list .= "</td>\n\t<td>";
	$man_list .= "<input type=\"text\" name=\"name[$id_com]\" value=\"$name\" size=\"30\" class=\"sgtext\">";
	$man_list .= "<button type=\"submit\" "; 
	$man_list .= 	 "value=\"$id_com\" ";
	$man_list .= 	"name=\"update\">Update</button>";
	$man_list .= "</td>\n\t<td style=\"text-align:center;\">";
	$man_list .= "<button type=\"submit\" "; 
	$man_list .= 	 "value=\"$id_com\" ";
	$man_list .= 	"name=\"delete\">Delete</button>";
	$man_list .= "</td>\n</tr>\n";

}


?>
<html>
<head>
<title>Manufacturer Compendium<? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Compendium of Manufacturers</h2>
<form name="delete_form" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<input type="hidden" name="come_from" value="hey_there">
<table width="98%" class="sansgrid">
<tr>
	<th>Man. ID</th>
	<th>Manufacturer Name</th>
	<th>Delete?</th>
</tr>
<? print $man_list; ?>
</table>
</form>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
