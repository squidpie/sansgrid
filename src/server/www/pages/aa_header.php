<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

function get_header() {
	global $SG;
	$url = $SG['project_url'];

	// If the server requires mating verification, then we'll have to add a 
	// link to the mating verification page. 
	$db = returnDatabaseConnection();
	$query = "SELECT verify_mating FROM server WHERE id_server= 1";
	$result = mysqli_query($db, $query) or die ("Couldn't execute query HDR1");
	$row = mysqli_fetch_assoc($result);
	$verify_mating = $row['verify_mating'];


	// If we do require mating verification, is anyone waiting to mate?
	// If so, $style_override will override the css stylesheet to set the logo
	// to an animated gif indicating the mating request. 
	$style_override = "";
	if ($verify_mating == 1) {

		$db = returnDatabaseConnection();
		$query = "SELECT COUNT(*) as count FROM pipeline WHERE latest_tx = 'Peck'";
		$result = mysqli_query($db, $query) or die ("Couldn't execute query HDR2");
		$row = mysqli_fetch_assoc($result);
		$count = $row['count'];
		if ($count > 0 ) {

			$style_override .= "<style>";
			$style_override .= "nav ul li#logo a {\n";
			$style_override .= "\tbackground:url(/img/mating.gif) 0px 0px;\n";
			$style_override .= "\tbackground-repeat:no-repeat;\n";
			$style_override .= "}\n\n";
			$style_override .= "nav ul li#logo:hover a {\n";
			$style_override .= "\tbackground:url(/img/mating.gif) 0px -100px;\n";
			$style_override .= "\tbackground-repeat:no-repeat;\n";
			$style_override .= "}\n";
			$style_override .= "</style>\n";
		}
	}

	$msg  = "<!-- Auto-generated by aa_header.php -->\n";
	$msg .= $style_override;
	$msg .= "<div id=\"navbar\">\n";
	$msg .= "\t<nav>\n";
	$msg .= "\t<ul>\n";
	$msg .= "\t\t<li id=\"logo\"><a href=\"$url\"></a></li>\n";
	$msg .= "\t\t<li><a href=\"#\">Sensors</a>\n";
	$msg .= "\t\t\t<ul>\n";
	$msg .= "\t\t\t\t<li><a href=\"$url/pages/sensors.php\">Known Sensors</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"$url/pages/com.php\">Manufacturer<br>Compendium</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"$url/pages/cos.php\">Sensor<br>Compendium</a></li>\n";
	$msg .= "\t\t\t</ul>\n";
	$msg .= "\t\t</li>\n";
	$msg .= "\t\t<li><a href=\"#\">Logs</a>\n";
	$msg .= "\t\t\t<ul>\n";
	$msg .= "\t\t\t\t<li><a href=\"$url/pages/logs.php\">Logs</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"$url/pages/pipeline.php\">Pipeline</a></li>\n";
	$msg .= "\t\t\t</ul>\n";
	$msg .= "\t\t</li>\n";
	$msg .= "\t\t<li><a href=\"#\">Tools</a>\n";
	$msg .= "\t\t\t<ul>\n";
	$msg .= "\t\t\t\t<li><a href=\"#\">Settings</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"#\">Triggers</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"#\">Admin</a>\n";
	$msg .= "\t\t\t\t\t<ul>\n";
	$msg .= "\t\t\t\t\t\t<li><a href=\"$url/pages/admin_server.php\">Server</a></li>\n";
	$msg .= "\t\t\t\t\t\t<li><a href=\"$url/pages/admin_router.php\">Routers</a></li>\n";
	$msg .= "\t\t\t\t\t\t<li><a href=\"#\">Users</a></li>\n";
	$msg .= "\t\t\t\t\t</ul>\n";
	if ($verify_mating == 1) 
		$msg .= "\t\t\t\t<li><a href=\"$url/pages/mate.php\">Mate</a></li>\n";
	$msg .= "\t\t\t</ul>\n";
	$msg .= "\t\t</li>\n";
	$msg .= "\t\t<li><a href=\"#\">Help</a>\n";
	$msg .= "\t\t\t<ul>\n";
	$msg .= "\t\t\t\t<li><a href=\"#\">User's Guide</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"#\">Programmer's Guide</a></li>\n";
	$msg .= "\t\t\t\t<li><a href=\"http://sansgridpdx.com\">SansGrid Wiki</a></li>\n";
	$msg .= "\t\t\t</ul>\n";
	$msg .= "\t\t</li>\n";
	$msg .= "\t</ul>\n";
	$msg .= "\t</nav>\n";
	$msg .= "</div>\n\n";

	return $msg;
}
