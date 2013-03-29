<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$router_list = "";

// Is the page-load a response to a form entry?  If so, which one?
if ( isset ($_POST["come_from"])) {

	switch ($_POST["come_from"]) {

		case "add_router": 
			addRouter();
			break;

		case "delete_router":
			deleteRouter();
			break;
	}

}

// Get list of known routers
$query = "SELECT * FROM router ORDER BY id_router";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {
	
	$router_list .= "<tr>\n\t<td>";
	$router_list .= $row["note"];
	$router_list .= "</td>\n\t<td>";
	$router_list .= $row["router_key"];
	$router_list .= "</td>\n\t<td>";
	$router_list .= "<button type=\"submit\" "; 
	$router_list .= 	 "value=\"" . $row["id_router"] . "\" ";
	$router_list .= 	"name=\"delete\">Delete?</button>";
	$router_list .= "</td>\n</tr>\n";
}


// ***********************************************************************************************************
// addRouter() - Adds router from $_POST data via web form
function addRouter() {
	global $db; 

	// Just being paranoid
	$note = isset ($_POST["note"]) ? $_POST["note"] : "''";
	$router_key = isset ($_POST["hashkey"]) ? $_POST["hashkey"] : "";

	if ($note != "") {
		$query = "INSERT INTO router (note, router_key) VALUES ('$note', '$router_key')";
	} else {
		$query = "INSERT INTO router (router_key) VALUES ('$router_key')";
	}
	$result = mysqli_query ($db, $query) or die ("Couldn't execute query 2.<br>$query");
}

// ***********************************************************************************************************
// deleteRouter() - Deletes from database using $_POST data via web form
function deleteRouter() {
	global $db; 

	if ( isset ($_POST["delete"]) && $_POST["delete"] != "") {
		$id_router = $_POST["delete"];
		$query = "DELETE FROM router WHERE id_router = '$id_router'";
		$result = mysqli_query ($db, $query) or die ("Couldn't execute query 3.");
	}
}

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

<h2>Add New Router</h2>
<form name="add_form" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<input type="hidden" name="come_from" value="add_router">
<table>
	<tr>
		<td> Note (optional): </td>
		<td> <input type="textbox" name="note" size="30em" maxlength="250"> </td>
	</tr>
	<tr>
		<td> Router key: </td>
		<td> <input type="textbox" name="hashkey" size="65em" readonly value="<? print generateRandomHash(64);?>"> </td>
	</tr>
	<tr>
		<td> </td>
		<td> <input type="submit" value="Add router"> </td>
	</tr>
</table>
</form>

<hr>

<h2>Known Routers</h2>
<form name="delete_form" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<input type="hidden" name="come_from" value="delete_router">
<table width="98%" class="sansgrid">
<tr>
	<th width="40%">Notes</th>
	<th width="50%">Router Keys</th>
	<th>Delete?</th>
</tr>
<? print $router_list; ?>
</table>
</form>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
