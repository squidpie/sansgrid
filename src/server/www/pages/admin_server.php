<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");

$db = returnDatabaseConnection();

$query = "SELECT * FROM server WHERE id_server = 1";

$result = mysqli_query($db, $query) or die ("Couldn't execute query 1.");
$row = mysqli_fetch_assoc($result);

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
	<th class="left">Server Key:</th>
	<td colspan=2><? print $row['server_key']; ?></td>
</tr>
<tr>
	<th width="200px" class="left">Automatic Mating:</th>
	<td><? print $row['verify_mating'] == 0 ? "Disabled" : "Enabled"; ?></td>
	<td style="text-align: right;"><b>[toggle]</b></td>
</tr>
</table>


</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
