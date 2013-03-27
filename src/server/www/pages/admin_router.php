<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");
?>
<html>
<head>
<link rel="stylesheet" type="text/css" href="<? print $project_url;?>/style.css"> 
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Add New Router</h2>
<form method="post" action="">
	Note (optional):
	<input type="textbox" name="note" size="30em">
	<input type="textbox" name="hashkey" size="30em" readonly value="">
</form>
<h2>Known Routers</h2>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
