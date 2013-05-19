<?
/*
 This file is a gateway file.  The purpose of super_include.php is to include
 any other files that may be needed within the project.  
*/


# $_SERVER["DOCUMENT_ROOT"] resolves to "/var/www" in a raspbian/lighttpd setup
include_once($_SERVER["DOCUMENT_ROOT"] . "/config.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/aa_header.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/zz_logger.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/zz_api_functions.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/zz_pipeline.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/zz_sensor.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/zz_tools.php");
include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/zz_trigger.php");
#include_once($_SERVER["DOCUMENT_ROOT"] . "/pages/aa_style.php");

// Let's take advantage of the fact this page is loaded with any other page and
// run some basic housekeeping here. :D
cleanPipeline();


?>
