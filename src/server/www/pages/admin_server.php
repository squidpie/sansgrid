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
$refresh_rate 	= $row['refresh_rate'];

// Options for the dropdown list of refresh rates.
$dropdown_options[0] = "&#x221E;";
$dropdown_options[1] = "1";
$dropdown_options[2] = "2";
$dropdown_options[3] = "3";
$dropdown_options[4] = "4";
$dropdown_options[5] = "5";
$dropdown_options[6] = "15";
$dropdown_options[7] = "30";
$dropdown_options[8] = "45";
 

// Is the page-load a response to a form submission?
if ( isset ($_POST) ) {
	
	// Automatic mating
	if ( isset ($_POST['automate']) ) {
		// Whatever the current setting is for verify_mating, toggle it 
		$new_mating = $verify_mating == 1 ? 0 : 1;
		
		$query = "UPDATE server SET verify_mating='$new_mating'";
		mysqli_query($db, $query) or die ("Couldn't execute query 2.");

		// Update the variable for display on the page. 
		$verify_mating = $new_mating;
	}
	
	// Automatic page reloads
	if ( isset ($_POST['do_refresh']) ) {

		// Get refresh rate from the array, but...
		$refresh_rate =  $dropdown_options[$_POST['refresh_rate']];
		// ...set the infinity sign back to zero
		$refresh_rate = $refresh_rate == "&#x221E;" ? 0 : $refresh_rate;


		
		$query = "UPDATE server SET refresh_rate='$refresh_rate'";
		mysqli_query($db, $query) or die ("Couldn't execute query 3.");

		/*
		// Whatever the current setting is for verify_mating, toggle it 
		$new_mating = $verify_mating == 1 ? 0 : 1;
		
		$query = "UPDATE server SET verify_mating='$new_mating'";
		mysqli_query($db, $query) or die ("Couldn't execute query 2.");

		// Update the variable for display on the page. 
		$verify_mating = $new_mating;
		*/
	}
	
}

// The dropdown list for th refresh rate is dynamically created so that the
// current refresh rate is selected when the page is loaded.
$query = "SELECT refresh_rate FROM server";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");
$row = mysqli_fetch_assoc($result);
$refresh_rate = $row['refresh_rate'];

// Loop through the dropdown_options and whichever matches what's in the 
// database gets selected.
$dropdown  = "\t<select name=\"refresh_rate\">\n";
foreach ($dropdown_options as $key => $value) {
	
	if ($refresh_rate == $value) {
		$dropdown .= "\t\t<option value=\"$key\" selected>$value</option>";
	} else {
		$dropdown .= "\t\t<option value=\"$key\">$value</option>";
	}
}
$dropdown .= "\t</select>\n";


?>
<html>
<head>
<title>Server Setup - <? print $SG['project_title'];?></title>
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
		<button type="submit" name="automate">&larr;&nbsp;Toggle?</button>
	</td>
</tr>
<tr>
	<th width="200px" class="left">Auto Refresh:</th>
	<td>
		Every 
		<? print $dropdown; ?>
		seconds.
	</td>
	<td style="text-align: right;">
		<form name="confirm_mate" method="post" 
			action="<?php echo $_SERVER['PHP_SELF']; ?>" >
		<button type="submit" name="do_refresh">Update</button>
	</td>
</tr>
</table>


</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
