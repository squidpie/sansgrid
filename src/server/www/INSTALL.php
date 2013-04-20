<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

// Establish connection with server.
$db = @mysqli_connect("$SG_domain", "$SG_db_user", "$SG_db_pass") or die ("Couldn't connect to database.");

// Check for existence of "sansgrid" database. If it exists we should delete it. 
if ( mysqli_select_db($db, 'sansgrid') ) {
	// Database exists
	print "The \"sansgrid\" database already exists. If you wish to begin a new installation please ";
	print "delete it and run INSTALL.php again.";
	// UNCOMMENT THIS!!!  UNCOMMENT THIS!!!  UNCOMMENT THIS!!!  UNCOMMENT THIS!!!  UNCOMMENT THIS!!! 
	//die ("<br>");

	// DELETE THIS!!!  DELETE THIS!!!  DELETE THIS!!!  DELETE THIS!!!  DELETE THIS!!!  DELETE THIS!!!
	$query = "DROP DATABASE sansgrid";
	$result = mysqli_query($db, $query) or die ("Couldn't delete database, quitting.");
	print "<br><br>I just blindly deleted the sansgrid database.  <span style=\"color: red;\">This is terrible</span><br><br>";

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
$query = "CREATE TABLE server (id_server int NOT NULL UNIQUE AUTO_INCREMENT, server_key VARCHAR(250), verify_mating int(1) )";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'server', quitting.<br>$query");

// Table: router
$query = "CREATE TABLE router (id_router int NOT NULL UNIQUE AUTO_INCREMENT, router_key VARCHAR(250), note VARCHAR(250))";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'router', quitting.");

// Table: sensor
$query = "CREATE TABLE sensor (id_sensor int NOT NULL UNIQUE AUTO_INCREMENT, modnum int, manid int, sn int, status VARCHAR(250), last_pulse TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'sensor', quitting.");

// Table: io
$query = "CREATE TABLE io (id_io int NOT NULL UNIQUE AUTO_INCREMENT, id_sensor int, direction VARCHAR(10), value VARCHAR(250), last_edit TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'io', quitting.");


// Table: log
$query = "CREATE TABLE log (id_log INT NOT NULL UNIQUE AUTO_INCREMENT, log VARCHAR(1024), time timestamp)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'log', quitting.");


// Table: pipeline
$query = "CREATE TABLE pipeline (id_sensor int; rdid varchar(50), latest_tx varchar(10), last_update timestamp)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'pipeline', quitting.");


// Generate and save server_id and set other system defaults
$server_key = generateRandomHash(128);	// 128 characters = 64 bytes
$server_id  = generateRandomHash(32);	// 32 characters = 16 bytes
$query = "INSERT INTO server (server_id, server_key, verify_mating) VALUES ('$server_id', '$server_key', 0)";
$result = mysqli_query($db, $query) or die ("Couldn't initialize new server.");


mysqli_close($db);
?>
<html>
<head>
<title>SansGrid Installer</title>
</head>
<body>

<p>
<b>There should be some text here indicating that we were successful or whatever</b>
</p>

</body>
</html>
