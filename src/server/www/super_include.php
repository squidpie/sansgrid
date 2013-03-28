<?
/*
 This file is a gateway file.  The purpose of super_include.php is to include
 any other files that may be needed within the project.  
*/


# $_SERVER["DOCUMENT_ROOT"] resolves to "/var/www" in a raspbian/lighttpd setup
include_once($_SERVER["DOCUMENT_ROOT"] . "config.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "pages/aa_header.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "pages/zz_tools.php");
#include_once($_SERVER["DOCUMENT_ROOT"] . "pages/aa_style.php");

?>
