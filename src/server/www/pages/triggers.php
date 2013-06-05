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

// ****************************************************************************
// Is the page-load a response to a form entry?

// Is this a delete...
if (isset ($_POST["delete"])) {

	$id_trigger = $_POST["delete"];
	$query = "DELETE FROM triggers WHERE id_trigger='$id_trigger'";
	mysqli_query($db, $query) or die ("Can't execute query 13\n");
}

// ... or is this an add?
if (isset ($_POST["add"])) {

	// That's right, the only error checking in the entire server happens
	// right here. 
	$errors = 0;

	$id_src_sensor 	= $_POST["source_sensor"];
	$id_src_signal 	= $_POST["source_signal"];
	$id_dest_sensor = $_POST["destination_sensor"];
	$id_dest_signal = $_POST["destination_signal"];
	$trigger_type 	= $_POST["trigger_type"];
	$dest_type 		= $_POST["dest_type"];

	$trigger_value 	= $_POST["trigger_value"];
	// Blanks will be treated as 0
	$trigger_value = $trigger_value == "" ? "0" : $trigger_value;

	$dest_value 	= $_POST["dest_value"];
	// Stripping out numerical data
	$dest_value = preg_replace ("/[^0-9]*(\d+\.\d+).*$/U", "$1", $dest_value);
	$dest_value = preg_replace ("/[^0-9.]/", "", $dest_value);
	// Blanks will be treated as 0
	$dest_value = $dest_value == "" ? "0" : $dest_value;

	// Here's that error checking you were warned about.
	$errors = $id_src_sensor  == 0 		? ++$errors : $errors;
	$errors = $id_src_signal  == "null"	? ++$errors : $errors;
	$errors = $id_dest_sensor == 0 		? ++$errors : $errors;
	$errors = $id_dest_signal == "null" ? ++$errors : $errors;
	$errors = $trigger_type   == "null"	? ++$errors : $errors;


	/*
	print "id_src_sensor: $id_src_sensor<br>\n";
	print "id_src_signal: $id_src_signal<br>\n";
	print "id_dest_sensor: $id_dest_sensor<br>\n";
	print "id_dest_signal: $id_dest_signal<br>\n";
	print "trigger_type: $trigger_type<br>\n";
	print "trigger_value: $trigger_value<br>\n";
	print "dest_type: $dest_type<br>\n";
	print "dest_value: $dest_value<br>\n";
	print "errors: $errors<br>\n";
	*/

	$query  = "INSERT INTO triggers ";
	$query .= "(id_src_sensor, id_src_signal, id_dest_sensor, id_dest_signal, ";
	$query .= " trigger_type, trigger_value, dest_type, dest_value) ";
	$query .= " VALUES ('$id_src_sensor', '$id_src_signal', '$id_dest_sensor', ";
	$query .= " '$id_dest_signal', '$trigger_type', '$trigger_value', ";
	$query .= " '$dest_type', '$dest_value') ";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query 7.<br>$query");
}

// ****************************************************************************
// This section covers the "Add New Trigger" form

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
				or die ("Couldn't execute query 3.");

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
$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_sensor = $row['id_sensor'];

	// Next we ensure that this sensor has successfully mated.
	$query2 = "SELECT * FROM sensor WHERE id_sensor='$id_sensor'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 5.");
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
				or die ("Couldn't execute query 6.");

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

// ****************************************************************************
// This section contains the code for listing existing triggers
$trigger_list = "";

// Get all triggers
$query = "SELECT * FROM triggers";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 8.");

while ($row = mysqli_fetch_assoc($result)) {

	$id_trigger 	= $row["id_trigger"];
	$id_src_sensor 	= $row["id_src_sensor"];
	$id_src_signal 	= $row["id_src_signal"];
	$id_dest_sensor = $row["id_dest_sensor"];
	$id_dest_signal = $row["id_dest_signal"];
	$trigger_type 	= $row["trigger_type"];
	$trigger_value 	= $row["trigger_value"];
	$dest_type 		= $row["dest_type"];
	$dest_value 	= $row["dest_value"];

	// Get source sensor name
	$query2 = "SELECT name FROM sensor WHERE id_sensor='$id_src_sensor'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 9.");
	$row2 = mysqli_fetch_assoc($result2);

	$src_sensor_name = $row2['name'] != "" ? $row2['name'] : "($id_src_sensor)";

	// Get destination signal name
	$query2  = "SELECT label FROM io ";
	$query2 .= "WHERE id_sensor='$id_src_sensor' AND id_io='$id_src_signal'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 10.");
	$row2 = mysqli_fetch_assoc($result2);

	$src_signal_name = $row2['label'] != "" ? $row2['label'] : "-";

	// Get destination sensor name
	$query2 = "SELECT name FROM sensor WHERE id_sensor='$id_dest_sensor'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 11.");
	$row2 = mysqli_fetch_assoc($result2);

	$dest_sensor_name = $row2['name'] != "" ? $row2['name'] : "($id_dest_sensor)";

	// Get destination signal name
	$query2  = "SELECT label FROM io ";
	$query2 .= "WHERE id_sensor='$id_dest_sensor' AND id_io='$id_dest_signal'";
	$result2 = mysqli_query($db, $query2) or die ("Couldn't execute query 12.");
	$row2 = mysqli_fetch_assoc($result2);

	$dest_signal_name = $row2['label'] != "" ? $row2['label'] : "-";

	// Building the table data

	// Source
	$trigger_list .= "<tr>\n";
	$trigger_list .= "\t<th rowspan=3>\n";
	$trigger_list .= "\t\t($id_trigger)\n";
	$trigger_list .= "\t</th>\n";
	$trigger_list .= "\t<td>\n";
	$trigger_list .= "\t\tSrc:\n";
	$trigger_list .= "\t</td>\n";
	$trigger_list .= "\t<td>\n";
	$trigger_list .= "\t\t$src_sensor_name&rarr;$src_signal_name\n";
	$trigger_list .= "\t</td>\n";

	// Source triggers
	$trigger_list .= "\t<td>\n";
	switch ($trigger_type) {
		case "digital_0":
			$trigger_list .= "\t\tis a digital 0\n";
			break;

		case "digital_1":
			$trigger_list .= "\t\tis a digital 1\n";
			break;

		case "digital_change":
			$trigger_list .= "\t\thas changed\n";
			break;

		case "greater_than":
			$trigger_list .= "\t\t> $trigger_value\n";
			break;

		case "greater_than":
			$trigger_list .= "\t\t> $trigger_value\n";
			break;

		case "less_than":
			$trigger_list .= "\t\t< $trigger_value\n";
			break;

		case "equal":
			$trigger_list .= "\t\t= $trigger_value\n";
			break;

	}
	$trigger_list .= "\t</td>\n";
	$trigger_list .= "</tr>\n";

	// Destination
	$trigger_list .= "<tr>\n";
	$trigger_list .= "\t<td>\n";
	$trigger_list .= "\t\tDest:\n";
	$trigger_list .= "\t</td>\n";
	$trigger_list .= "\t<td>\n";
	$trigger_list .= "\t\t$dest_sensor_name&rarr;$dest_signal_name\n";
	$trigger_list .= "\t</td>\n";
	// Source triggers
	$trigger_list .= "\t<td>\n";
	switch ($dest_type) {
		case "digital_0":
			$trigger_list .= "\t\t send a 0\n";
			break;

		case "digital_1":
			$trigger_list .= "\t\t send a 1\n";
			break;

		case "trigger_value":
			$trigger_list .= "\t\tsend trigger value\n";
			break;

		case "user_value":
			$trigger_list .= "\t\tsend '$dest_value'\n";
			break;

	}
	$trigger_list .= "</tr>\n";
	$trigger_list .= "<tr>\n";
	$trigger_list .= "\t<td colspan=3 style=\"text-align: right;\">\n";
	$trigger_list .= "\t\t<button type=\"submit\" "; 
	$trigger_list .= 	"value=\"$id_trigger\" ";
	$trigger_list .= 	"name=\"delete\">Delete Trigger #$id_trigger</button>\n";
	$trigger_list .= "\t</td>\n";
	$trigger_list .= "</tr>\n";

}

// ****************************************************************************
?>
<html>
<head>
<title>Trigger - <? print $SG['project_title'];?></title>
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
		tmp_obj.value = "digital_0";
		tmp_obj.text = "is 0"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = "digital_1";
		tmp_obj.text = "is 1"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = "digital_change";
		tmp_obj.text = "has changed"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.getElementById("trigger_value");
		tmp_obj.style.display = "none";



	// source_type = 1: Analog
	} else {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = "equal";
		tmp_obj.text = "="; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = "greater_than";
		tmp_obj.text = ">"; 
		trigger_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = "less_than"; 
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
	output_dropdown = document.getElementById("dest_type");
	while (output_dropdown.firstChild) {
		output_dropdown.removeChild(output_dropdown.firstChild);
	}


	// Fill in dropdown appropriately
	// destination_type = 0: Digital 
	if ( destination_type == 0) {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = "digital_0";
		tmp_obj.text = "a 0"; 
		output_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = "digital_1";
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
			tmp_obj.value = "trigger_value";
			tmp_obj.text = "the trigger"; 
			output_dropdown.appendChild(tmp_obj);
		}

		tmp_obj = document.getElementById("dest_value");
		tmp_obj.style.display = "none";



	// destination_type = 1: Analog
	} else {
		var tmp_obj = document.createElement("option");
		tmp_obj.value = "user_value";
		tmp_obj.text = "the value:"; 
		output_dropdown.appendChild(tmp_obj);

		tmp_obj = document.createElement("option");
		tmp_obj.value = "trigger_value";
		tmp_obj.text = "the trigger"; 
		output_dropdown.appendChild(tmp_obj);

		tmp_obj = document.getElementById("dest_value");
		tmp_obj.style.display = "inline";

	}

}

// *****************************************************************************


function showDestValueBox() {
		// Value of the dropbox
		tmp  = document.getElementById("dest_type");
		dest_type = tmp.options[tmp.selectedIndex].value;

		// Reference to textbox
		text_obj = document.getElementById("dest_value");

		if (dest_type == "user_value") {
			text_obj.style.display = "inline";
		} else {
			text_obj.style.display = "none";
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
<? //print returnRefresh(); ?>
</head>

<body onLoad="resetForm();">
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Add New Trigger</h2>
<form name="add_form" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<input type="hidden" name="come_from" value="hey_there">

<table width="98%">
<tr>
	<td width="45%">
		<!-- Choose sensor as source of trigger -->
		<h3>Source:</h3>
		<select name="source_sensor" id="source_sensor" onChange="updateSourceSignals(this.value);">
		<? print $source_sensor_list; ?>
		</select>
	</td>

	<td width="10%">
	</td>

	<td width="45%">
		<!-- Choose sensor as destination of trigger -->
		<h3>Destination:</h3>
		<select name="destination_sensor" id="destination_sensor" onChange="updateDestinationSignals(this.value);">
		<? print $dest_sensor_list; ?>
		</select>
	</td>
</tr>
<tr>
	<td colspan=3 class="gen_padding">
		When 
		<!-- Choose signal for this source sensor -->
		<select name="source_signal" id="source_signal" onChange="setTriggerCondition();">
		<option value="null">--</option>
		</select>

		<!-- Choose trigger type for this source signal -->
		<select name="trigger_type" id="trigger_type">
		<option value="null">--</option>
		</select>
		<input type="textbox" name="trigger_value" id="trigger_value" size=4 style="display: none;">,

		send
		<!-- Choose signal for the destination sensor -->
		<select name="destination_signal" id="destination_signal" onChange="setTriggerOutputMenu();">
		<option value="null">--</option>
		</select>

		<!-- Choose data type for the destination signal -->
		<select name="dest_type" id="dest_type" onChange="showDestValueBox();">
		<option value="null">--</option>
		</select>
		<input type="textbox " name="dest_value" id="dest_value" size=4 style="display: none;">.
	</td>
</tr>
<tr>
	<td colspan=3 class="gen_padding" style="text-align: right;">
		<button type="submit" name="add" value="add">Add new trigger</button>
	</td>
</tr>
</table>
</form>

<hr>

<h2>Current Triggers</h2>
<form name="delete_form" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>" >
<table class="sansgrid">
<? print $trigger_list; ?>
</table>
</form>


</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
