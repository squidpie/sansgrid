<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

// Get server data
$query = "SELECT * FROM server WHERE id_server = 1";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");
$row = mysqli_fetch_assoc($result);

$server_id 		= $row['server_id'];
$server_key 	= $row['server_key'];
$verify_mating 	= $row['verify_mating'];


// Is the page-load a response to a form submission?
if ( isset ($_POST['toggle']) ) {
	
	// Whatever the current setting is for verify_mating, toggle it to the other
	$new_mating = $verify_mating == 1 ? 0 : 1;
	
	$query = "UPDATE server SET verify_mating='$new_mating'";
	mysqli_query($db, $query) or die ("Couldn't execute query 2.");

	// Update the variable for display on the page. 
	$verify_mating = $new_mating;
	
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
<h2>Server Information</h2>
<table class="sansgrid">
<tr>
	<th class="left">Server ID:</th>
	<td colspan=2><? print $server_id; ?></td>
</tr>
<tr>
	<th class="left">Server Key:</th>
	<td colspan=2><? print $server_key; ?></td>
</tr>
<tr>
	<th width="200px" class="left">Mating Verification:</th>
	<td><? print $verify_mating == 1 ? "Required" : "Automatic"; ?></td>
	<td style="text-align: right;">
		<form name="confirm_mate" method="post" 
			action="<?php echo $_SERVER['PHP_SELF']; ?>" >
		<button type="submit" name="toggle">&larr; Toggle?</button>
	</td>
</tr>
</table>


</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
