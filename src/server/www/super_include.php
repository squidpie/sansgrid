<?
/*
 This file is a gateway file.  The purpose of super_include.php is to include
 any other files that may be needed within the project.  
*/


# $_SERVER["DOCUMENT_ROOT"] resolves to "/var/www" in a raspbian/lighttpd setup

#include_once($_SERVER["DOCUMENT_ROOT"] . "mypath/my2ndpath/myfile.php");

?>
