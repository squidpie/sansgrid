<?
$hostname = "localhost";		// MySQL hostname
$username = "sansgrid";			// MySQL user name
$password = "psu";				// MySQL password for above user

// Establish connection with server.
$db = @mysqli_connect("$localhost", "$username", "$password") or die ("Couldn't connect to database.");

// Check for existence of "sansgrid" database. If it exists we should delete it. 
if ( mysqli_select_db($db, 'sansgrid') ) {
	// Database exists
	print "The \"sansgrid\" database already exists. If you wish to begin a new installation please ";
	print "delete it and run INSTALL.php again.";
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
#$query = "CREATE TABLE server (id_server int() NOT NULL UNIQUE PRIMARY AUTO_INCREMENT, server_name VARCHAR(250))";

// Table: router
$query = "CREATE TABLE router (id_router int , router_key VARCHAR(250), note VARCHAR(250))";
#$query = "CREATE TABLE 'router' ('id_router' int , 'router_key' VARCHAR(250), 'note' VARCHAR(250))";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'router', quitting.<br>$query");

// Table: sensor
$query = "CREATE TABLE sensor (id_sensor int NOT NULL UNIQUE AUTO_INCREMENT, modnum int, manid int, status VARCHAR(250), last_pulse TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'sensor', quitting.<br>$query");

// Table: io
$query = "CREATE TABLE io (id_io int NOT NULL UNIQUE AUTO_INCREMENT, id_sensor int, direction VARCHAR(10), value VARCHAR(250), last_edit TIMESTAMP)";
$result = mysqli_query($db, $query) or die ("Couldn't create table 'sensor', quitting.<br>$query");


mysqli_close($db);
?>
<html>
<head>
<title>SansGrid</title>
</head>
<body>

<p>
<b>There should be some text here indicating that we were successful or whatever</b>
</p>

</body>
</html>
