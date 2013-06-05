<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
$db = returnDatabaseConnection();

$status = "";
$man_list = "";

// Is this an update of the sensor name?
if (isset ($_POST["update"])) {

	$id_sensor 	= $_POST["update"];
	$name 		= $_POST["name"][$id_sensor];
	
	$query = "UPDATE sensor SET name='$name' WHERE id_sensor='$id_sensor'";
	mysqli_query($db, $query) or die ("Couldn't execute query 1.");
}

// Is this an update of the sensor name?
if (isset ($_POST["send"])) {
	$pieces = explode ("-", $_POST["send"]);
	$an_or_dig = $pieces[0];		// Analog or digital data?
	$id_sensor = $pieces[1];
	$id_signal = $pieces[2];

	// Get the sig_id for this id_signal
	$query = "SELECT sig_id FROM io WHERE id_io=$id_signal";
	$result = mysqli_query($db, $query) 
		or die ("Error: Couldn't execute query 2.");
	$row = mysqli_fetch_assoc($result);
	$sig_id = $row['sig_id'];

	// Analog data
	if ($an_or_dig == "an") {
		$data =  $_POST["an-data"][$id_sensor];
		// Stripping out numerical data
		$data = preg_replace ("/[^0-9]*(\d+\.\d+).*$/U", "$1", $data);
		$data = preg_replace ("/[^0-9.]/", "", $data);
		// Blank entry will be treated as a 0.
		$data = ($data == "") ? "0" : $data;

	// Digital data
	} else {
		$data =  $_POST["di-data"][$id_sensor];
	}

	// Now we need the router's ip address and rdid tied to this sensor
	$query  = "SELECT router_ip, rdid, status FROM sensor ";
	$query .= "WHERE id_sensor=$id_sensor";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query 3.");
	$row = mysqli_fetch_assoc($result);

	$router_ip 	= $row['router_ip']; 
	$rdid 		= $row['rdid']; 
	$status		= $row['status']; 

	if ($status == "offline") 
		return;
	
	sendChirpData ($rdid, $sig_id, $data);
}



//  Pretty much all of the html for the entire page goes into $page
$page = "<form name=\"sgform\" method=\"post\" action=\"${_SERVER['PHP_SELF']}\" >\n";

// Get list of all sensors we've mated with that are online
$query  = "SELECT * FROM sensor WHERE has_mated='y'";
$query .= " AND status='online' or status='stale'";
$query .= " ORDER BY manid, modnum";
$result = mysqli_query($db, $query) or die ("Couldn't execute query 4.");

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
	$page .= "<legend class=\"$status\">$sensor_name</legend>";
	
	// Do we have any inputs to the sensor?...
	$query  = "SELECT COUNT(*) AS count FROM io ";
	$query .= " WHERE id_sensor='$id_sensor' AND direction='1'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 5.");
	$row2 = mysqli_fetch_assoc($result2);

	//...If so, let's list them. 
	if ($row2['count'] > 0) {
		$page .= "<h4>Sensor Inputs</h4>";

		$page .= "<table width=\"98%\" class=\"sansgrid\">";

		$query  = "SELECT * FROM io ";
		$query .= " WHERE id_sensor='$id_sensor' AND direction='1'";
		$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 6.");
		while ($row2 = mysqli_fetch_assoc($result2)) {

			$id_io = $row2['id_io'];
			$label = $row2['label'];
			$units = $row2['units'];
			$class = $row2['class'];

			$page .= "<tr>\n";
			$page .= "\t<th width=\"150px\">$label:</th>\n";
			$page .= "\t<td>\n";

			// Analog
			if ($class == 1 ) {
				$page .= "\t\t<input type=\"text\" name=\"an-data[$id_sensor]\""; 
				$page .=       " size=\"15\" maxlength=79 class=\"sgtext\">";
				$page .=       " &nbsp;$units";
			// Digital 
			} else {
				//$page .= "\t\t$units:&nbsp;&nbsp; ";
				$page .= "\t\t ";
				$page .= "<input type=\"radio\" name=\"di-data[$id_sensor]\" ";
				$page .= " value=\"0\" checked> 0 &nbsp;&nbsp;&nbsp;&nbsp; ";
				$page .= "<input type=\"radio\" name=\"di-data[$id_sensor]\" ";
				$page .= " value=\"1\" checked> 1";
			}
			$page .= "\t</td>\n";
			$page .= "\t<td width=\"6em\" style=\"text-align:center;\">\n";


			if ($status != "stale") {
				// The 'Send' button transmits information regarding the 
				// id_sensor, the id_io, and the class (analog or digital).  This 
				// is used to determine where to send the data (id_sensor, id_io) 
				// as well as which type of data we're processing 
				// (analog = textbox, digital = radio buttons). 
				$page .= "\t\t<button type=\"submit\" "; 
				// Analog
				if ($class == 1 ) {
					$page .= 	"value=\"an-$id_sensor-$id_io\" ";
				// Digital 
				} else {
					$page .= 	"value=\"di-$id_sensor-$id_io\" ";
				}
				$page .= 	"name=\"send\">Send</button>\n";

			} else {
				$page .= 	"<i>(stale)</i>\n";
			}

			$page .= "\t</td>\n";
		}

		$page .= "</table>";
	}

	// Do we have any outputs from the sensor?...
	$query  = "SELECT COUNT(*) AS count FROM io ";
	$query .= " WHERE id_sensor='$id_sensor' AND direction='0'";
	$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 7.");
	$row2 = mysqli_fetch_assoc($result2);

	//...If so, let's list them. 
	if ($row2['count'] > 0) {
		$page .= "<h4>Sensor Outputs</h4>";

		$page .= "<table width=\"98%\" class=\"sansgrid\">";

		$query  = "SELECT * FROM io ";
		$query .= " WHERE id_sensor='$id_sensor' AND direction='0'";
		$result2 = mysqli_query($db, $query) or die ("Couldn't execute query 8.");
		while ($row2 = mysqli_fetch_assoc($result2)) {

			$class = $row2['class'];
			$label = $row2['label'];
			$units = $row2['units'];
			$value = $row2['value'] != "" ? $row2['value'] : "-";

			$page .= "<tr>\n";
			$page .= "\t<th width=\"150px\">$label:</th>\n";

			// Analog
			if ($class == 1 ) {
				$page .= "\t<td>$value $units</td>\n";
			} else {
				$page .= "\t<td>$value</td>\n";
			}
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
<? //print returnRefresh(); ?>
<meta http-equiv="refresh" content="5">
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
