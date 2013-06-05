<?
include_once($_SERVER["DOCUMENT_ROOT"] . "super_include.php");


?>
<html>
<head>
<title>Logs - <? print $SG['project_title'];?></title>
<link rel="stylesheet" type="text/css" href="<? print $SG['project_url'];?>/style.css"> 
<? print returnRefresh(); ?>
</head>

<body>
<div id="page">

<? print get_header(); ?>

<div id="main">

<h2>Logs</h2>
<? print returnLogTable(); ?>

</div> 			<!-- end of <div id="main"> -->
</div> 			<!-- end of <div id="page"> -->
</body>
</html>
