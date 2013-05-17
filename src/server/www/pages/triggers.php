<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$source_sensor_list		= "\t<option value=\"0\">Choose a Sensor</option>\n";
$dest_sensor_list 		= "\t<option value=\"0\">Choose a Sensor</option>\n";
$destination_signal_list = "";

// These are meta variables, if you were.  They're variables that will contain 
// the text required to create and populate javascript variables used to make
// dynamic changes to the dropdown lists in the Add New Trigger form
$js_source_signal_id_list    = "src_sig_ids = new Array();\n";
$js_source_signal_label_list = "src_sig_label = new Array();\n";
$js_source_signal_class_list = "src_sig_class = new Array();\n";
$js_destination_signal_id_list    = "dest_sig_ids = new Array();\n";
$js_destination_signal_label_list = "dest_sig_label = new Array();\n";
$js_destination_signal_class_list = "dest_sig_class = new Array();\n";

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

} // End of form reponses

// ****************************************************************************

// Get list of all sensors that have inputs (output from server)
$query = "SELECT DISTINCT(id_sensor) FROM io WHERE direction='0'";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_sensor = $row['id_sensor'];

	// Next we ensure that this sensor has successfully mated.
	$query2 = "SELECT * FROM sensor WHERE id_sensor='$id_sensor'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 2.");
	$row2 = mysqli_fetch_assoc($result2);

	$has_mated	= $row2['has_mated'];
	$name		= ($row2['name'] != "") ? $row2['name'] : "($id_sensor)";

	// If it has then...  
	if ($row2['has_mated'] == "y") {

		// ...first we add it to the dropdown box...
		$source_sensor_list .= "\t<option value=\"$id_sensor\">$name</option>\n";

		// ...next we get all of the input signals and add it to the
		// javascript array for dynamic signal updates in the html form
		$query2  = "SELECT * FROM io ";
		$query2 .= "WHERE id_sensor='$id_sensor' AND direction='0'";
		$result2 = mysqli_query($db, $query2) 
				or die ("Couldn't execute query 2.");

		// Create new, second-dimension arrays within the javascript arrays
		$tmp = "  src_sig_label[$id_sensor] = new Array();\n";
		$js_source_signal_label_list .= $tmp;

		$tmp = "  src_sig_class[$id_sensor] = new Array();\n";
		$js_source_signal_class_list .= $tmp;

		while ($row2 = mysqli_fetch_assoc($result2) ) {

			$id_io	= $row2['id_io'];
			$label	= $row2['label'];
			$class	= $row2['class'];

			$tmp = "    src_sig_label[$id_sensor][$id_io] = \"$label\";\n";
			$js_source_signal_label_list .= $tmp;

			$tmp = "    src_sig_class[$id_sensor][$id_io] = $class;\n";
			$js_source_signal_class_list .= $tmp;
		}

	}
}

// Now we get list of all sensors that have outputs (input from server)
$query = "SELECT DISTINCT(id_sensor) FROM io WHERE direction='1'";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 3.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_sensor = $row['id_sensor'];

	// Next we ensure that this sensor has successfully mated.
	$query2 = "SELECT * FROM sensor WHERE id_sensor='$id_sensor'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 2.");
	$row2 = mysqli_fetch_assoc($result2);

	$has_mated	= $row2['has_mated'];
	$name		= ($row2['name'] != "") ? $row2['name'] : "($id_sensor)";

	// If it has then...  
	if ($row2['has_mated'] == "y") {

		// ...first we add it to the dropdown box...
		$dest_sensor_list .= "\t<option value=\"$id_sensor\">$name</option>\n";

		// ...next we get all of the output signals and add it to the
		// javascript array for dynamic signal updates in the html form
		$query2  = "SELECT * FROM io ";
		$query2 .= "WHERE id_sensor='$id_sensor' AND direction='1'";
		$result2 = mysqli_query($db, $query2) 
				or die ("Couldn't execute query 2.");

		// Create new, second-dimension arrays within the javascript arrays
		$tmp = "  dest_sig_label[$id_sensor] = new Array();\n";
		$js_destination_signal_label_list .= $tmp;

		$tmp = "  dest_sig_class[$id_sensor] = new Array();\n";
		$js_destination_signal_class_list .= $tmp;

		while ($row2 = mysqli_fetch_assoc($result2) ) {

			$id_io	= $row2['id_io'];
			$label	= $row2['label'];
			$class	= $row2['class'];

			$tmp = "    dest_sig_label[$id_sensor][$id_io] = \"$label\";\n";
			$js_destination_signal_label_list .= $tmp;

			$tmp = "    dest_sig_class[$id_sensor][$id_io] = $class;\n";
			$js_destination_signal_class_list .= $tmp;
		}

	}
}


?>
<html>
<head>
<title><? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 

<script>
<? 
// Here are our javascript variables
print $js_source_signal_label_list; 
print "\n";
print $js_source_signal_class_list; 
print "\n";
print $js_destination_signal_label_list; 
print "\n";
print $js_destination_signal_class_list; 
?>

// When a new sensor is picked as the source for the trigger, this function
// updates the signal list for that particular sensor.
function updateSourceSignals(new_sensor) {

	source_signal_dropdown = document.getElementById("source_signal");

	// Empty current sensor dropdown
	while (source_signal_dropdown.firstChild) {
		source_signal_dropdown.removeChild(source_signal_dropdown.firstChild);
	}

	// Now populate the dropdown with input signals this sensor.
	for ( key in src_sig_label[new_sensor] ) {

		// Populate the dropdown
		var tmp_obj = document.createElement("option");
		tmp_obj.value = key;
		tmp_obj.text = src_sig_label[new_sensor][key]; 
		source_signal_dropdown.appendChild(tmp_obj);
	}

	// Lastly, let's update the trigger type to match whatever signal is
	// currently selected
	setTriggerCondition();
}

// *****************************************************************************

// When a new sensor is picked as the destination for the trigger, this function
// updates the signal list for that particular sensor.
function updateDestinationSignals(new_sensor) {

	destination_signal_dropdown = document.getElementById("destination_signal");

	// Empty current sensor dropdown
	while (destination_signal_dropdown.firstChild) {
		destination_signal_dropdown.removeChild(destination_signal_dropdown.firstChild);
	}

	// Now populate the dropdown with input signals this sensor.
	for ( key in dest_sig_label[new_sensor] ) {

		// Populate the dropdown
		var tmp_obj = document.createElement("option");
		tmp_obj.value = key;
		tmp_obj.text = dest_sig_label[new_sensor][key]; 
		destination_signal_dropdown.appendChild(tmp_obj);
	}

	// Lastly, let's update the trigger type to match whatever signal is
	// currently selected
	setTriggerOutputMenu()
}


// *****************************************************************************


// Digital signals and analog signals have different options for trigger
// conditions.  This function updates the dropbox to reflect that.
function setTriggerCondition() {

	// First get the sensor
	tmp  = document.getElementById("source_sensor");
	id_sensor = tmp.options[tmp.selectedIndex].value;

	// Next get the signal
	tmp  = document.getElementById("source_signal");
	id_signal = tmp.options[tmp.selectedIndex].value;

	// Lastly, figure out what the source type is
	source_type = src_sig_class[id_sensor][id_signal];

	// Empty current trigger dropdown
	trigger_dropdown = document.getElementById("trigger_type");
	while (trigger_dropdown.firstChild) {
		trigger_dropdown.removeChild(trigger_dropdown.firstChild);
	}


	// Fill in dropdown appropriately
	// source_type = 0: Digital 
	if ( source_type == 0) {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = 0;
		tmp_obj.text = "is 0"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = 1;
		tmp_obj.text = "is 1"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = 2;
		tmp_obj.text = "has changed"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.getElementById("trigger_value");
		tmp_obj.style.display = "none";



	// source_type = 1: Analog
	} else {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = 3;
		tmp_obj.text = "="; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = 4;
		tmp_obj.text = ">"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = 5;
		tmp_obj.text = "<"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.getElementById("trigger_value");
		tmp_obj.style.display = "inline";

	}

	// Because the destination trigger affected, in part, by the source trigger
	// we should update the menu just in case.
	setTriggerOutputMenu();

} // End setTriggerCondition()


// *****************************************************************************


// Digital signals and analog signals can recieve different trigger responses.
// This function updates the dropbox to reflect that.
function setTriggerOutputMenu() {

	// First get the sensor
	tmp  = document.getElementById("destination_sensor");
	id_sensor = tmp.options[tmp.selectedIndex].value;

	// Next get the signal
	tmp  = document.getElementById("destination_signal");
	id_signal = tmp.options[tmp.selectedIndex].value;

	// Lastly, figure out what the destination type is
	destination_type = dest_sig_class[id_sensor][id_signal];

	// Empty current output dropdown
	output_dropdown = document.getElementById("send_type");
	while (output_dropdown.firstChild) {
		output_dropdown.removeChild(output_dropdown.firstChild);
	}


	// Fill in dropdown appropriately
	// destination_type = 0: Digital 
	if ( destination_type == 0) {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = 0;
		tmp_obj.text = "a 0"; 
		output_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = 1;
		tmp_obj.text = "a 1"; 
		output_dropdown.appendChild(tmp_obj);

		// If (and only if!) the trigger source is digital, then we add the 
		// option of adding the value of the trigger
		// First get the sensor
		tmp  = document.getElementById("source_sensor");
		id_sensor = tmp.options[tmp.selectedIndex].value;

		// Next get the signal
		tmp  = document.getElementById("source_signal");
		id_signal = tmp.options[tmp.selectedIndex].value;

		// Lastly, figure out what the source type is
		source_type = src_sig_class[id_sensor][id_signal];

		// If the source is digital, then we can send the value as the output
		if ( source_type == 0) {
			tmp_obj = document.createElement("option");
			tmp_obj.value = 2;
			tmp_obj.text = "the trigger"; 
			output_dropdown.appendChild(tmp_obj);
		}

		tmp_obj = document.getElementById("send_value");
		tmp_obj.style.display = "none";



	// destination_type = 1: Analog
	} else {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = 3;
		tmp_obj.text = "the value:"; 
		output_dropdown.appendChild(tmp_obj);

		tmp_obj = document.getElementById("send_value");
		tmp_obj.style.display = "inline";

	}

}


// *****************************************************************************


function resetForm () {
	var tmp = document.getElementById("source_sensor");
	tmp.options[0].selected = true;

	tmp = document.getElementById("source_signal");
	tmp.options[0].selected = true;

	var tmp = document.getElementById("destination_sensor");
	tmp.options[0].selected = true;

	tmp = document.getElementById("destination_signal");
	tmp.options[0].selected = true;
}

function testing () {
	//var mebbe = document.getElementById("source_sensor");
	//mebbe.options[0].selected = true;

}


</script>

<!-- DELETE THIS LINE BELOW !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! -->
<? print returnRefresh(); ?>
</head>

<body onLoad="resetForm();">
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Add New Trigger</h2>
<form name="delete_form" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<input type="hidden" name="come_from" value="hey_there">

<table width="98%">
<tr>
	<td width="45%">
		<h3>Source:</h3>
		<select id="source_sensor" onChange="updateSourceSignals(this.value);">
		<? print $source_sensor_list; ?>
		</select>
	</td>

	<td width="10%">
	</td>

	<td width="45%">
		<h3>Destination:</h3>
		<select id="destination_sensor" onChange="updateDestinationSignals(this.value);">
		<? print $dest_sensor_list; ?>
		</select>
	</td>
</tr>
<tr>
	<td colspan=3 class="gen_padding">
		When 
		<select name="source_signal" id="source_signal" onChange="setTriggerCondition();">
		<option value="null">--</option>
		</select>

		<select name="trigger_type" id="trigger_type">
		<option value="null">--</option>
		</select>
		<input type="textbox " name="trigger_value" id="trigger_value" size=4 style="display: none;">,

		send
		<select name="destination_signal" id="destination_signal" onChange="setTriggerOutputMenu();">
		<option value="null">--</option>
		</select>

		<select name="send_type" id="send_type">
		<option value="null">--</option>
		</select>
		<input type="textbox " name="send_value" id="send_value" size=4 style="display: none;">.
	</td>
</tr>
<tr>
	<td colspan=3 class="gen_padding" style="text-align: right;">
		<button type="submit" value="add">Add new trigger</button>
	</td>
</tr>
</table>
</form>

<hr>


</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
