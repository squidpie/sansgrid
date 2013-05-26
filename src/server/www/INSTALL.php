<?
include_once($_SERVER["DOCUMENT_ROOT"] . "config.php");

// Establish connection with server.
$db = @mysqli_connect($SG['domain'], $SG['db_user'], $SG['db_pass']) or die ("Couldn't connect to database.");

// Check for existence of "sansgrid" database. If it exists we should delete it. 
if ( mysqli_select_db($db, 'sansgrid') ) {
	// Database exists
	print "The \"sansgrid\" database already exists. If you wish to begin a new installation please ";
	print "delete it and run INSTALL.php again.";
	die ("<br>");
}



// Create database
$query = "CREATE DATABASE sansgrid";
$result = mysqli_query($db, $query) or die ("Couldn't create database, quitting.");

// Connect (or reconnect) to database
mysqli_select_db($db, 'sansgrid') or die ("Can't re-select database...");

if ($db->connect_errno) {
	echo "Failed to connect to MySQL: (" 
		. $db->connect_errno . ") " . $db->connect_error;
}

// Table: server
$query = "CREATE TABLE server (id_server int NOT NULL UNIQUE AUTO_INCREMENT, server_id VARCHAR(250), server_key VARCHAR(250), verify_mating INT(1), refresh_rate INT )";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'server', quitting.<br>$query");

// Table: router
$query = "CREATE TABLE router (id_router int NOT NULL UNIQUE AUTO_INCREMENT, router_key VARCHAR(250), note VARCHAR(250))";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'router', quitting.");

// Table: sensor
$query = "CREATE TABLE sensor (id_sensor int NOT NULL UNIQUE AUTO_INCREMENT, manid VARCHAR(10), modnum VARCHAR(10), sn VARCHAR(20), sensor_key VARCHAR(150), name VARCHAR(250), status VARCHAR(250), has_mated CHAR(1) DEFAULT 'n', router_ip VARCHAR(50), last_pulse TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'sensor', quitting.");

// Table: io
$query = "CREATE TABLE io (id_io int NOT NULL UNIQUE AUTO_INCREMENT, id_sensor INT, sig_id INT, class VARCHAR(15), direction VARCHAR(5), label VARCHAR(50), units VARCHAR(10), value VARCHAR(250), last_edit TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'io', quitting.");

// Table: log
$query = "CREATE TABLE log (id_log INT NOT NULL UNIQUE AUTO_INCREMENT, log VARCHAR(1024), time timestamp)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'log', quitting.");

// Table: pipeline
$query = "CREATE TABLE pipeline (id_sensor INT, rdid VARCHAR(50), latest_tx VARCHAR(10), router_ip VARCHAR(50), workspace VARCHAR(250), last_update TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'pipeline', quitting.");

// Table: com (Compendium of Manufacturers) 
$query = "CREATE TABLE com (id_com INT NOT NULL UNIQUE AUTO_INCREMENT, manid VARCHAR(10), name VARCHAR(250))";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'com', quitting.");

// Table: cos (Compendium of Sensors) 
$query = "CREATE TABLE cos (id_cos INT NOT NULL UNIQUE AUTO_INCREMENT, manid VARCHAR(10), modnum VARCHAR(10), name VARCHAR(250))";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'cos', quitting.");

// Table: triggers
$query = "CREATE TABLE triggers(id_trigger INT NOT NULL UNIQUE AUTO_INCREMENT, id_src_sensor INT, id_src_signal INT, id_dest_sensor INT, id_dest_signal INT, trigger_type VARCHAR(50), trigger_value VARCHAR(250), dest_type VARCHAR(50), dest_value VARCHAR(250))";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'trigger', quitting.");


// Generate and save server_id and set other system defaults
$server_key = generateRandomHash(128);	// 128 characters = 64 bytes
$server_id  = generateRandomHash(32);	// 32 characters = 16 bytes
$query = "INSERT INTO server (server_id, server_key, verify_mating, refresh_rate) VALUES ('$server_id', '$server_key', 0, 0)";
$result = mysqli_query($db, $query) or die ("Couldn't initialize new server.");

/* ************************************************************************** */
// Returns a random hex string that's $length characters long
function generateRandomHash ($length) {
	$tmp = "";
	for ($i = 0; $i < $length; ++$i) {
		$tmp .= sprintf ("%x", mt_rand(0,15));
	}


	return $tmp;
}
/* ************************************************************************** */

mysqli_close($db);
?>
<html>
<head>
<title>SansGrid Installer</title>
</head>
<body>

<p>
Congratulations! Your SansGrid server is fully installed. It is <b>highly</b>
recommended that you delete this file (INSTALL.php). 
</p>

</body>
</html>
